/*  
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h> 
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_ASM_TYPES_H
#include <asm/types.h> 
#endif
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif 
#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif
#include <getopt.h>
#include <assert.h>
#ifdef HAVE_BITS_SOCKADDR_H
#include <bits/sockaddr.h>
#endif
#include <netdb.h>
#include <string.h>

#ifdef HAVE_LIBWRAP
#include <tcpd.h>
#include <syslog.h>
#endif

#ifdef HAVE_ANSIDECL_H
#include <ansidecl.h>
#endif
#ifdef __STDC__
#include <stdarg.h>
#else
#include <varargs.h>
#endif
#define HAVE_TIMEZONE
#define HAVE_SYSCONF 1
#define HAVE_FCNTL_LOCK 1

#ifndef ENOTSOCK
#define ENOTSOCK EINVAL
#endif

#ifndef SIGCLD
#define SIGCLD SIGCHLD
#endif 

#ifndef GETWD_CACHE
#define GETWD_CACHE 0
#endif

#ifndef HAVE_FCNTL_LOCK
#define HAVE_FCNTL_LOCK 1
#endif

