/* utils.c is part of tgc package. 
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

#ifndef __FUNCTION__
#define __FUNCTION__    "?"
#endif

#define BACKLOG         8

int log_level = 0;
int is_it_daemon = 0;

/* log file descriptor */
FILE	*logfile = NULL;

#ifdef HAVE_LIBWRAP
int     allow_severity = LOG_INFO;
int     deny_severity = LOG_WARNING;
#endif

/*------------------------------------------------------------------
  Opens the logfile and sets the loglevel
------------------------------------------------------------------*/
void init_log(int dlevel, char *log_filename, int daemon)
{
#ifdef DEBUG
	if (log_filename && log_filename[0])
		logfile = fopen(log_filename, "a");
	else 
		close_log();
	
	if (logfile)
		setbuf(logfile, NULL);

	if (dlevel>=0)
		log_level = dlevel;

	is_it_daemon = daemon;
#endif
}

/*------------------------------------------------------------------
  Closes the log file
------------------------------------------------------------------*/
void close_log(void)
{
#ifdef DEBUG
	if (logfile)
		fclose(logfile);
	logfile = NULL;
#endif
}

/*------------------------------------------------------------------
  Writes a log message to the log file, and on the stderr if not daemon
------------------------------------------------------------------*/
#ifdef __STDC__
void print_log(int dlevel, const char *format_str, ...)
{
#else
void print_log(va_alist)
va_dcl
{
 	int  dlevel;
	const char *format_str;
#endif
#ifdef DEBUG
	char log_msg[256];
	char time_buf[64];
	time_t now;
	va_list	ap;
	char err_str[64];
	struct timeval tv;

	//if (!logfile) return;

	if (dlevel<=0)
		return;

	err_str[0]='\0';

#ifdef __STDC__
	va_start(ap, format_str);
#else
	va_start(ap);
	dlevel = va_arg(ap,int);
	format_str = va_arg(ap,char *);
#endif
	if (dlevel <= log_level) {
		//now = time(NULL);
		gettimeofday(&tv, NULL);
		now = tv.tv_sec;
		strftime(time_buf, 64, "%Y/%m/%d %T", localtime(&now));
		vsnprintf(log_msg, 256, format_str, ap);

		if (errno && log_level>5) {
			sprintf(err_str, " errno=%d(%s)", errno, strerror(errno)); 
			errno=0;
		}

		if (logfile) {
			fprintf(logfile, "%s.%ld:: %s%s\n", time_buf, tv.tv_usec, log_msg, err_str);
			fflush(logfile);
		}

		if (!is_it_daemon) 
			fprintf(stderr, "%s.%ld:: %s%s\n", time_buf, tv.tv_usec, log_msg, err_str);
	}
	va_end(ap);

#endif
}
/*------------------------------------------------------------------
  Writes a log message to the log file, and on the stderr if not daemon
------------------------------------------------------------------*/
#ifdef __STDC__
void print_log_msg(int dlevel, const char *func, const char *format_str, ...)
{
#else
void print_log_msg(va_alist)
va_dcl
{
 	int  dlevel;
	char *func;
	const char *format_str;
#endif
#ifdef DEBUG
	char tmp_msg[256];
	char log_msg[256];
	va_list	ap;

	//if (!logfile) return;

	if (dlevel<=0)
		return;

#ifdef __STDC__
	va_start(ap, format_str);
#else
	va_start(ap);
	dlevel = va_arg(ap,int);
	format_str = va_arg(ap,char *);
#endif
	vsnprintf(tmp_msg, 256, format_str, ap);
	if (func) {
		snprintf(log_msg, 256, "%s: %s", func, tmp_msg);
		print_log(dlevel, log_msg);
	} else 
		print_log(dlevel, tmp_msg);

	va_end(ap);

#endif
}

/*-----------------------------------------------------------------------------
    To prevent zombie child processes.
------------------------------------------------------------------------------*/
RETSIGTYPE sig_cld()
{
       //signal(SIGCLD, sig_cld);
       //while (waitpid((pid_t)-1, (int *) NULL, WNOHANG) > 0);

	while (wait3((int *)NULL, WNOHANG, (struct rusage *)NULL) > 0);
}

/*---------------------------------------------------------------------------
   true if the machine is big endian
---------------------------------------------------------------------------*/
int big_endian(void)
{
	int x = 2;
	char *s;
	s = (char *)&x;

	return(s[0] == 0);
}

/*------------------------------------------------------------------
  compare 2 strings 
-------------------------------------------------------------------*/
int strequal(char *s1, char *s2)
{
	if (!s1 || !s2) 
		return(0);
	    
	return(strcasecmp(s1,s2)==0);
}

/*------------------------------------------------------------------
 convert a string to lower case
------------------------------------------------------------------*/
void strlower(char *s)
{
	while (*s) {
		*s = tolower(*s);
	        s++;
        }
}

/*------------------------------------------------------------------
  convert a string to upper case
------------------------------------------------------------------*/
void strupper(char *s)
{
	while (*s) {
		*s = toupper(*s);
	        s++;
	}
}

/*-----------------------------------------------------------------
  string replace
-----------------------------------------------------------------*/
void string_replace(char *s,char old_str, char new_str)
{
	while (*s) {
		if (old_str == *s)
	 	   	*s = new_str;
	        s++;
	}
}

/*-----------------------------------------------------------------
  string replace
-----------------------------------------------------------------*/
int strnum(char *s)
{
	while (*s) {
		if (!isdigit(*s)) 
			return 0;
		s++;
	}
	return 1;
}

/*-----------------------------------------------------------------
  Become a daemon !
-----------------------------------------------------------------*/
void become_daemon(void)
{
#ifdef HAVE_FORK
	if (fork())
		exit(0);	//parent exits

	// we are the child now!

        /* Stop zombies */
        signal(SIGCLD, sig_cld);
	
#ifdef SETPGRP_VOID
	setpgrp();
#else
	setpgrp(getpid(), 0);
#endif
	setsid();

	close(0);
	close(1);
	close(2);

	if (chdir("/"))
		; // the 'if' is there to supress the unused-return-value-wanring
#endif
}

/*-----------------------------------------------------------------------------
  Change the effective user.
------------------------------------------------------------------------------*/
void change_user()
{
        seteuid(getuid());
        setegid(getgid());
}

/*-----------------------------------------------------------------
  xioctl, just a layer on top of ioctl to give us more info
  in case of errors!
-----------------------------------------------------------------*/
int xioctl(int fd, int request, void *argp)
{
	int	rv;

	do 
		rv = ioctl(fd, request, argp);
	while (rv==-1  &&  EINTR==errno);

#ifdef DEBUG
	if (rv == -1) 
		print_log(3, "ioctl error");
#endif

	return rv;
}

/*-----------------------------------------------------------------
 Calculate the difference in timeout values. Return 1 if val1 > val2,
 0 if val1 == val2, -1 if val1 < val2. Stores result in retval. retval
 may be == val1 or val2
-----------------------------------------------------------------*/
int tval_sub( struct timeval *retval, struct timeval *val1, struct timeval *val2)
{
	long usecdiff = val1->tv_usec - val2->tv_usec;
	long secdiff = val1->tv_sec - val2->tv_sec;
	if(usecdiff < 0) {
		usecdiff = 1000000 + usecdiff;
		secdiff--;
	}
	retval->tv_sec = secdiff;
	retval->tv_usec = usecdiff;
	if(secdiff < 0)
		return -1;
	if(secdiff > 0)
		return 1;
	return (usecdiff < 0 ) ? -1 : ((usecdiff > 0 ) ? 1 : 0);
}

/*-----------------------------------------------------------------
  read data from peer
-----------------------------------------------------------------*/
int read_data(int fd, unsigned char *buf, int len)
{
        int  nread=0;
        int  total_read = 0;

	if (fd<0) return -1;

        while (total_read < len) {
                nread = read(fd, buf+total_read, len-total_read);

                if (nread < 0)
                        return -1;

		if (nread==0)
			return total_read;

                total_read += nread;
        }
        return total_read;
}

/*-----------------------------------------------------------------------------
  Parse host:port string
------------------------------------------------------------------------------*/
int parse_host(char *hostport, char *host, unsigned short *port)
{
        char    *hp, *t;
        int     rc = 0;

        if (!hostport || !host || !port)
                return -1;

        hp = strdup(hostport);

        if ( (t=strtok(hp, ":")) ) {
		rc = 0;
                strncpy(host, t, MAX_PATH);
                if ( (t=strtok(NULL, ":")) )
                        if ( !(*port=atoi(t)) )
                		rc = -1;
        } else
                rc = -1;

        if (hp)
                free(hp);

        return rc;
}

/*-----------------------------------------------------------------------------
 * open client socket!
------------------------------------------------------------------------------*/
int connect_server(char *host, int port)
{
        struct sockaddr_in sock;
        int    sd=0, one=1;
        struct hostent  *host_ent;

        if ( !(host_ent = gethostbyname(host)) ) {
                fprintf(stderr, "unknown host %s\n", host);
                return -1;
        }

        memset(&sock, 0, sizeof(sock));
        sock.sin_port = htons(port);
        sock.sin_family = AF_INET;
        memcpy(&sock.sin_addr.s_addr, host_ent->h_addr, 4);
        if ( (sd = socket(PF_INET, SOCK_STREAM, 0)) < 0 ) {
                PRINT_LOG(1, "socket failed");
                return -1;
        }

        if ( connect(sd ,(struct sockaddr *)&sock, sizeof(sock)) < 0 ) {
                PRINT_LOG(1, "failed connecting to %s:%d", host, port);
                close(sd);
                return -1;
        }

        setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));

        return sd;
}


/*-----------------------------------------------------------------------------
 * open server socket!
------------------------------------------------------------------------------*/
int open_server_socket(int port)
{
        struct sockaddr_in addr;
        int    sd=0, one=1;
        //int    in_addrlen = sizeof(addr);

        memset(&addr, 0, sizeof(addr));
        addr.sin_port = htons( port );
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        sd = socket(PF_INET, SOCK_STREAM, 0);
        if (sd == -1)  {
                PRINT_LOG(1,"socket failed");
                return -1;
        }

        setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &one, sizeof(one));
	setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

        /* now we've got a socket - we need to bind it */
        if (bind(sd, (struct sockaddr * ) &addr, sizeof(addr)) < 0) {
                PRINT_LOG(1,"bind failed");
                close(sd);
                return -1;
        }

	if (listen(sd, BACKLOG) == -1) {
                PRINT_LOG(1, "failed to listen");
                close(sd);
                return -1;
        }

	return sd;
}

/*-----------------------------------------------------------------------------
 * Check to see weather we should allow access to a host (using host_access)
------------------------------------------------------------------------------*/
#ifdef HAVE_LIBWRAP
int	peer_ok(char *prog, int sd)
{
	struct request_info request;

	request_init(&request, RQ_DAEMON, prog, RQ_FILE, sd, 0);
	fromhost(&request);
	if (!hosts_access(&request)) 
		return 0;
	else
		return 1;
}
#endif


/*-----------------------------------------------------------------------------
 * Accept a new connection and take care of the error codes
------------------------------------------------------------------------------*/
int accept_connection(int sd_accept, struct sockaddr_in *addr, socklen_t *in_addrlen)
{
	int sd;

	if (!addr || !in_addrlen)
		return -1;
	
	sd = accept(sd_accept, (struct sockaddr *) addr, (socklen_t *)in_addrlen);

	if (sd==-1) 
		PRINT_LOG(1, "Error accepting new connection!");

#ifdef HAVE_LIBWRAP
	if (sd>0 && !peer_ok(PACKAGE, sd)) {
		PRINT_LOG(2, "Rejecting %s!", inet_ntoa(addr->sin_addr));
		close(sd);
		sd = -1;
	}
#endif

	return sd;
}

/*-----------------------------------------------------------------------------
 * shuts down and closes the socket
------------------------------------------------------------------------------*/
void close_connection(int *sd)
{
	if (sd && *sd > 0) {
		shutdown(*sd, SHUT_RDWR);
		close(*sd);
		*sd = -1;
	}
}

