/* tgcd.c is part of tgc package.
   Copyright (C) 2014	Faraz.V (faraz@fzv.ca)
  
   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.
  
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

Disclaimer:
   This program is provided with no warranty of any kind, either expressed or
   implied.  It is the responsibility of the user (you) to fully research and
   comprehend the usage of this program.  As with any tool, it can be misused,
   either intentionally or unintentionally.
   THE AUTHOR(S) IS(ARE) NOT RESPONSIBLE FOR ANYTHING YOU DO WITH THIS PROGRAM
   or anything that happens because of your use (or misuse) of this program.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
   IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
   ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
   FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
   DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
   OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
   OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
   SUCH DAMAGE.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

#include "includes.h"
#include "utils.h"
#include "tgc.h"

int initial_uid = 0;
int initial_gid = 0;

int	timeout = 250;

const char *program_name;

#ifdef HAVE_FORK
// We try to be a daemon by default 
int	is_daemon = 1;
#else
int	is_daemon = 0;
#endif

TGC	tgc;

/*------------------------------------------------------------------------------
   Prints version info.
--------------------------------------------------------------------------------*/
void print_version(void)
{
	printf("TCP Gender Changer, V%s Copyright (C) 2014 Faraz.V (faraz@fzv.ca)\n", VERSION);
}

/*------------------------------------------------------------------------------
   Prints usage info.
--------------------------------------------------------------------------------*/
void print_usage(int exit_code)
{
	print_version();
	printf("Usage: %s { -C | -L | -F }  options ... \n\n", PACKAGE);
	printf("Options are:\n\n");
	

	printf(" ConnectConnect mode: %s -C -s host:port -c host:port [-i n ] [-k n ] [ common options ]\n", PACKAGE);
	printf(" ConnectConnect :\n");
	printf(" -C, --ccnode 		    Become a CC (ConnectConnect) node.\n");
	printf(" -s, --server host:port     The host and port of the actual server\n");
	printf(" -c, --llhost host:port     The host and port of the ListenListen node.\n");
	printf(" -i, --interval seconds     Time interval to periodically report to LL (default: %ds).\n", TGC_TIMEOUT);
	printf(" -k, --key number	    Poorman's encryption (0-255, default: 0, means no encryption)\n");
	
	printf("\n");
	printf(" ListenListen mode: %s -L -p port  -q port  [-k n ] [ common options ...]\n", PACKAGE);
	printf(" -L, --llnode		    Become a LL (ListenListen) node.\n");
	printf(" -q, --llport number 	    The port to listen on for incomming connection from a CC node\n");
	printf(" -p, --port number 	    The port to listen on for incomming actual client connection\n");
	printf(" -k, --key number 	    Poorman's encryption (0-255, default: 0, means no encryption)\n");
	
	printf("\n");
	printf(" PortForwarder mode: %s -F -p port -s host:port [ common options ... ]\n", PACKAGE);
	printf(" -F, --lcnode		    Become a ListenConnect node, i.e. just a simple port forwarder\n");
	printf(" -p, --port  number 	    The port to listen on for incomming actual client connection\n");
	printf(" -s, --server host:port     The host and port of the actual server or a LL node\n");

	printf("\n");
	printf("Common options:\n");
        printf(" -m, --method { f | s }	    f: Fork  s: Select (default: s)\n");
        printf(" -f, --filter filter	    Optional argument to run filter on new connections, IP passed as argument\n");
        printf(" -l, --log file 	    Write logs to file. (default:'%s')\n", TGC_DEFAULT_LOGFILE);
	printf(" -g, --level number 	    Log level detail (default:%d).\n", TGC_DEFAULT_LOGLEVEL);
	printf(" -n, --nodaemon             Do not become daemon\n");
	printf(" -h, --help		    Display this.\n");
	printf(" -v, --version		    Display version number.\n\n");

	exit(exit_code);
}

/*-----------------------------------------------------------------------------
   Shutdown the server
------------------------------------------------------------------------------*/
RETSIGTYPE shutdown_server(int sig)
{
	PRINT_LOG(1, "Shutting down (signal:%d)", sig);
	tgc_shutdown(&tgc);
	exit(0);
}

/*-----------------------------------------------------------------------------
   Initialize the server data structures and variables.
------------------------------------------------------------------------------*/
void init_server(void)
{
	big_endian();

	if (tgc_post_init(&tgc)<0) {
		PRINT_LOG(1, "Failed to initialize!");
		print_usage(2);
	}

	initial_uid = geteuid();
	initial_gid = getegid();

	signal(SIGPIPE, SIG_IGN);

	signal(SIGTERM, shutdown_server);
	signal(SIGINT, shutdown_server);
	signal(SIGQUIT, shutdown_server);
	signal(SIGHUP, shutdown_server);

	if (tgc.method==TGC_METHOD_FORK) 
		signal(SIGCLD, sig_cld);
}

/*-----------------------------------------------------------------------------
  The Main function !
------------------------------------------------------------------------------*/
int main(int argc,char *argv[])
{
	int	loglevel     = TGC_DEFAULT_LOGLEVEL;
	int	next_option;
	extern	char *optarg;	/* getopt */
	extern  int  optind;	/* getopt */
	char	logfilename[MAX_PATH+1] = {0};
	char	temp[MAX_PATH] = {0};
	int	ntemp=0, rc=0;
	struct stat filter_stat;
	
	/* short options */
	const char *short_options = "Cs:c:i:Lq:p:Fk:m:f:l:g:nhv";
	
	/* long options */
	const struct option long_options[] = {
		{"ccnode",	0, NULL, 'C'},
		{"server",	1, NULL, 's'},
		{"llhost",	1, NULL, 'c'},
		{"interval",	1, NULL, 'i'},

		{"llnode",	0, NULL, 'L'},
		{"llport",	1, NULL, 'q'},
		{"port",	1, NULL, 'p'},

		{"lcnode",	0, NULL, 'F'},

		{"key",		1, NULL, 'k'},

		{"method",	1, NULL, 'm'},
		{"filter",	1, NULL, 'f'},

		{"log",		1, NULL, 'l'},
		{"level",	1, NULL, 'g'},
		{"nodaemon",	0, NULL, 'n'},
		{"help",	0, NULL, 'h'},
		{"version",	0, NULL, 'v'},
		{NULL, 		0, NULL, 0}
	};

	strncpy(logfilename, TGC_DEFAULT_LOGFILE, MAX_PATH);
	tgc_pre_init(&tgc);

	program_name = argv[0];

	do {
		next_option = getopt_long(argc, argv, short_options, long_options, NULL);

		switch (next_option) {
			case 'C':
				tgc.type=CCNODE;
				break;
			case 's':
				strncpy(temp, optarg, MAX_PATH);
				if (tgc.type==FNODE)
                                	parse_host(temp, tgc.node.pf.dst_host, &(tgc.node.pf.dst_port) );
				else
                                	parse_host(temp, tgc.node.cc.server, &(tgc.node.cc.server_port) );
				break;
			case 'c':
				strncpy(temp, optarg, MAX_PATH);
                                parse_host(temp, tgc.node.cc.ll_host, &tgc.node.cc.ll_port);
				break;
			case 'L':
				tgc.type=LLNODE;
				break;
			case 'q':
				ntemp = atoi(optarg);
				if (!strnum(optarg) || ntemp<=0 || ntemp>65535) {
					fprintf(stderr, "Invalid port number '%s'\n", optarg);
					exit(2);
				}
				tgc.node.ll.ll_port = ntemp;
				break;
			case 'F':
				tgc.type=FNODE;
				break;
			case 'p':
				ntemp = atoi(optarg);
				if (!strnum(optarg) || ntemp<=0 || ntemp>65535) {
					fprintf(stderr, "Invalid port number '%s'\n", optarg);
					exit(2);
				}
				if (tgc.type == FNODE)
					tgc.node.pf.port = ntemp;
				else
					tgc.node.ll.port = ntemp;
				break;
			case 'l':
				strncpy(logfilename, optarg, MAX_PATH);		
				break;
			case 'f':
				strncpy( tgc.filter, optarg, MAX_PATH);
				rc = stat(tgc.filter, &filter_stat);
				if (rc || !S_ISREG(filter_stat.st_mode)) {
					fprintf(stderr, "Invalid filter '%s'\n", optarg);
					exit(3);
				}
				break;
			case 'm':
				strncpy( &(tgc.method), optarg, 1);
				break;
			case 'g':
				ntemp = atoi(optarg);
				if (!strnum(optarg) || ntemp<0) {
					fprintf(stderr, "Invalid loglevel '%s'\n", optarg);
					exit(2);
				}
				loglevel = ntemp;
				break;
			case 'i':
				ntemp = atoi(optarg);
				if (!strnum(optarg) || ntemp<=0 || ntemp>65535) {
					fprintf(stderr, "Invalid interval number '%s'\n", optarg);
					exit(2);
				}
				tgc.node.cc.interval = ntemp;
				break;
			case 'k':
				ntemp = atoi(optarg);
				if (!strnum(optarg) || ntemp<0 || ntemp>255) {
					fprintf(stderr, "Invalid key '%s'\n", optarg);
					exit(2);
				}
				tgc.key = ntemp;
				break;
			case 'n':
				is_daemon = 0;
				break;
			case 'h':
				print_usage(0);
				break;
			case 'v':
				print_version();
				exit(0);
				break;
			case -1:
				/* done with options! */
				break;
			case '?':
			default:
				print_usage(1);
		}
	} while (next_option != -1);
	
	/* if there are non-option parameters printn usage and exit! */
	if (optind<argc || argc<=1) 
		print_usage(1);

	init_log(loglevel, logfilename, is_daemon);

	init_server();

#ifdef HAVE_FORK
	if (is_daemon)
		become_daemon();
#endif
	tgc_run(&tgc);

	close_log();
	return 0;
}

