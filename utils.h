/* utils.h is part of tgc package
   Copyright (C) 2008	Faraz.V (faraz@fzv.ca)
  
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

#ifndef __UTILS_H__
#define __UTILS_H__

#define DEBUG

#ifdef __cplusplus
  extern "C" {
#endif 

#ifndef MAX_PATH
#define  MAX_PATH	254
#endif

void init_log(int dlevel, char *dbg_filename, int daemon);

void close_log(void);

#ifdef __STDC__
void print_log(int dlevel, const char *format_str, ...);
#else
void print_log(va_alist);
#endif

#ifdef __STDC__
void print_log_msg(int dlevel, const char *func, const char *format_str, ...);
#else
void print_log_msg(va_alist);
#endif

#if __GNUC__ > 3
	#ifdef DEBUG
	#define	PRINT_LOG(l, ...)	print_log_msg((l), __FUNCTION__, __VA_ARGS__)
	#else
	#define	PRINT_LOG(l, ...)	
	#endif
#else
	#ifdef DEBUG
	#define	PRINT_LOG(l, args...)	print_log_msg((l), __FUNCTION__, args)
	#else
	#define	PRINT_LOG(l, args...)	
	#endif
#endif

int 	big_endian(void);
int 	strequal(char *s1, char *s2);
void 	strlower(char *s);
void 	strupper(char *s);
void 	string_replace(char *s, char old_str, char new_str);
int	strnum(char *s);
void 	become_daemon(void);
void 	change_user(void);
int 	xioctl(int fd, int request, void *argp);
int 	read_data(int fd, unsigned char *buf, int len);
int 	parse_host(char *hostport, char *host, unsigned short *port);
int 	connect_server(char *host, int port);
int	open_server_socket(int port);
void 	close_connection(int *sd);
RETSIGTYPE sig_cld();
#ifdef HAVE_LIBWRAP
int	peer_ok(char *prog, int sd);
#endif
int 	accept_connection(int sd_accept, struct sockaddr_in *addr, socklen_t *in_addrlen);

#ifdef __cplusplus
  }
#endif 

#endif

