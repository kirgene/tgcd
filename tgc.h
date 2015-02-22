/* tgc.h is part of tgc package.
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

#ifndef _TGC_H_
#define _TGC_H_

#define TGC_DEFAULT_LOGFILE	""
#define TGC_DEFAULT_LOGLEVEL	0
#define TGC_BUFFER_SIZE 	4096

// time intervals between each connectin attempt to LL
#define  TGC_TIMEOUT	40

#define	CC_SERVER	tgc->node.cc.server
#define CC_SERVER_PORT	tgc->node.cc.server_port
#define LL_HOST		tgc->node.cc.ll_host
#define CC_LL_PORT	tgc->node.cc.ll_port


#define	TGC_COMM_PING	'P'	
#define	TGC_COMM_CCC	'O'
#define TGC_COMM_CLOSE	'C'

#define TGC_METHOD_FORK		'F'
#define TGC_METHOD_SELECT	'S'

// to queue up the incoming connections to pair them up together later
typedef struct _socketq {
	int	sd;
	struct _socketq	*next;
} socket_queue;

// to hold connection pairs
typedef struct _sock_pair_list {
	int	sdi;			//internal (LL-CC)
	int	sdx;			//external (C/S-CC/LL)
	struct _sock_pair_list	*next;
	struct _sock_pair_list	*prev;
} socket_pair_list;

typedef enum { CCNODE, LLNODE, FNODE }	node_type;

typedef struct {
	char 		server[MAX_PATH];
	char		ll_host[MAX_PATH];
	unsigned short	server_port;
	unsigned short  ll_port;
	unsigned short	interval;
	unsigned char	protocol;
	int		control_sd;	// control connection
	socket_queue	*socketq;	//external (C/S-CC/LL)
} ccnode_type;

typedef struct {
	unsigned short	port;
	unsigned short  ll_port;
	unsigned char	protocol;
	int		control_sd;	// control connection
	socket_queue	*socketq;	//external (C/S-CC/LL)
} llnode_type;


typedef struct {
	char 		dst_host[MAX_PATH];
	unsigned short	dst_port;
	unsigned short  port;
	unsigned char	src_protocol;
	unsigned char	dst_protocol;
} fnode_type;


typedef struct {
	node_type	type;
	union {
		ccnode_type	cc;
		llnode_type	ll;
		fnode_type	pf;
	} node;
	int			sdi_accept;
	int			sdx_accept;
	socket_pair_list	*pairs;
	unsigned char   	buf[TGC_BUFFER_SIZE];
	unsigned char		key;
	char   			method;
	char   			filter[MAX_PATH + 1];
} TGC;


typedef struct {
	char	*name;
	char	*addr;
	struct  sockaddr_in *sin;
	int	sd;
} host_info;

/* OP Codes */

/*
Messege Structure:

0: OP Code (1B)
1: Msg. Len (2B)
2: DATA
*/

/*-----------------------------------------*/

/* Error codes  */
#define E_TGC_OK		0
#define E_TGC_NOCANDO		-1
#define E_TGC_BADOPCODE		-2
#define E_TGC_IE		-3
#define E_TGC_READ		-4
#define E_TGC_WRITE		-5
#define E_TGC_END		-6


/* function prototypes */
int tgc_pre_init(TGC *tgc);
int tgc_post_init(TGC *tgc);
void tgc_shutdown(TGC *tgc);

int tgc_run(TGC *tgc);
int tgc_cc(TGC *tgc);
int tgc_ll(TGC *tgc);
int tgc_pf(TGC *tgc);

#endif

