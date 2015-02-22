/* tgc.c is part of tgc package
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

/*-----------------------------------------------------------------------------
 Do the 1st phase initialization
 return value (0: okay, else: not okay)
-----------------------------------------------------------------------------*/
int tgc_pre_init(TGC *tgc)
{
	if (!tgc) {
		PRINT_LOG(1, "Internal Error!");
		exit(1);
		//return E_TGC_IE;
	}
	memset(tgc, 0, sizeof(TGC));
	
	return 0;
}

/*-----------------------------------------------------------------------------
 Do the 2nd phase initialization
 return value (0: okay, else: not okay)
-----------------------------------------------------------------------------*/
int tgc_post_init(TGC *tgc)
{
	int rc=0;

	if (!tgc) {
		PRINT_LOG(1, "Internal Error!");
		exit(1);
		//return E_TGC_IE;
	}

	tgc->method = toupper(tgc->method);
	if (tgc->method!=TGC_METHOD_FORK && tgc->method!=TGC_METHOD_SELECT)
		tgc->method = TGC_METHOD_SELECT; // default

#ifndef HAVE_FORK
	tgc->method = TGC_METHOD_SELECT;
#endif

	switch (tgc->type) {
                case CCNODE:
			if (!strlen(tgc->node.cc.server) || !strlen(tgc->node.cc.ll_host) ||
			    !tgc->node.cc.server_port || !tgc->node.cc.ll_port)
                        	rc = E_TGC_NOCANDO;

			if (tgc->node.cc.interval<1)
				tgc->node.cc.interval=TGC_TIMEOUT;

                        break;
                case LLNODE:
			if (!tgc->node.ll.port || !tgc->node.ll.ll_port)
                                rc = E_TGC_NOCANDO;
                        break;
                case FNODE:
			if (!strlen(tgc->node.pf.dst_host) || !tgc->node.pf.dst_port || !tgc->node.pf.port)
                        	rc = E_TGC_NOCANDO;

			tgc->key=0;	// encryption key has no effect on a F node
                        break;
                default:
                        rc = E_TGC_NOCANDO;
        }

	return rc;
}


/*-----------------------------------------------------------------------------
 Ask CC for a new connection
-----------------------------------------------------------------------------*/
int tgc_send(int sd, char cmd)
{
	if (write(sd, &cmd, 1)<1) {
		PRINT_LOG(3, "Failed sending connection request");
		return E_TGC_NOCANDO;
	}

	return E_TGC_OK;
}

/*-----------------------------------------------------------------------------
 Read internal command!
-----------------------------------------------------------------------------*/
int tgc_read(int sd, char *cmd)
{
	int rc = 0;

	if (!cmd)
		return E_TGC_IE;
	
	if ( (rc = read(sd, cmd, 1)) < 0) {
		PRINT_LOG(3, "Failed reading command");
		return E_TGC_READ;
	}

	if (!rc) 
		return E_TGC_END;
	
	return E_TGC_OK;
}

/*-----------------------------------------------------------------------------
 * Add an element to queue
-----------------------------------------------------------------------------*/
int tgc_add_queue(socket_queue **sq, int sd)
{
	socket_queue	*tq;

	if (sd<0)
		return E_TGC_NOCANDO;
	
	if ( !(tq=(socket_queue *) malloc(sizeof(socket_queue))) ) {
		PRINT_LOG(1, "out of memory!");
		return E_TGC_IE;
	}
	tq->sd = sd;

	if (*sq) 
		tq->next = (*sq);
	else // null
		tq->next = NULL;
	*sq = tq;

	return E_TGC_OK;
}

/*-----------------------------------------------------------------------------
 Remove an element from queue
-----------------------------------------------------------------------------*/
int  tgc_remove_queue(socket_queue **sq)
{
	int		sd;
	socket_queue	*tq;

	if (!*sq)
		return E_TGC_NOCANDO;
	
	tq = *sq;	
	sd = (*sq)->sd;
	(*sq) = tq->next;
	free(tq);

	return sd;
}

/*-----------------------------------------------------------------------------
 Add an element to the list
-----------------------------------------------------------------------------*/
int tgc_add_list(socket_pair_list **list, int sdi, int sdx)
{
	socket_pair_list	*pair;

	if (sdi<0 || sdx<0)
		return E_TGC_NOCANDO;
	
	if ( !(pair=(socket_pair_list *) malloc(sizeof(socket_pair_list))) ) {
		PRINT_LOG(1, "Out of memory!");
		return E_TGC_IE;
	}
	pair->sdi = sdi;
	pair->sdx = sdx;
	pair->prev = NULL;

	if (*list) {
		pair->next = *list;
		(*list)->prev = pair;
		*list = pair;
	} else {
		pair->next = NULL;
		*list = pair;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
 Remove an element from the list
-----------------------------------------------------------------------------*/
int tgc_remove_list(socket_pair_list **list, socket_pair_list *pair)
{
	if (!pair || !*list)
		return E_TGC_NOCANDO;

	if (pair->prev) {
		if (pair->next) { // middle node
			pair->prev->next = pair->next;
			pair->next->prev = pair->prev;
		} else { // end node
			pair->prev->next = NULL;
		}
	} else {
		if (pair->next) { // first node
			*list = pair->next;
			(*list)->prev = NULL;
		} else { //the only node
			(*list) = NULL;
		}
	}
	free(pair);

	return 0;
}

/*-----------------------------------------------------------------------------
 Shutdown, whatelse?!
-----------------------------------------------------------------------------*/
void tgc_shutdown(TGC *tgc)
{
	socket_queue	*sq=NULL;
	int		sd=-1;

	if (!tgc) return;

	if (tgc->type == CCNODE) {
		sq = tgc->node.cc.socketq;
		sd = tgc->node.cc.control_sd;
	} else if (tgc->type == LLNODE) {
		sq = tgc->node.ll.socketq;
		sd = tgc->node.cc.control_sd;
	}
	
	if (sd>=0) {
		close_connection(&sd);
	}

	if (tgc->sdi_accept>=0) {
		close_connection(&(tgc->sdi_accept));
	}

	if (tgc->sdx_accept>=0) {
		close_connection(&(tgc->sdx_accept));
	}
	
	while (sq) {
		sd = tgc_remove_queue(&sq);
		close_connection(&sd);
	}
	
	while (tgc->pairs) {
		close_connection(&(tgc->pairs->sdi));
		close_connection(&(tgc->pairs->sdx));
		tgc_remove_list(&(tgc->pairs), tgc->pairs );
	}
}

/*------------------------------------------------------------------------------
 *   Signal handler for SIGPIPE (write on a disconnected socket)
 *   --------------------------------------------------------------------------------*/
void do_abort(void )
{
        PRINT_LOG(1,"Aborted.");
        exit(1);
}
 
/*-----------------------------------------------------------------------------
 * Transfer data from one socket to another
------------------------------------------------------------------------------*/
int tgc_rxtx(int rx_sd, int tx_sd, unsigned char *buf, unsigned char key)
{
	int	nread=0, nwritten=0, i;

	if (rx_sd<0 || tx_sd<0 || !buf)
		return E_TGC_IE;

	if ( (nread=read(rx_sd, buf, TGC_BUFFER_SIZE)) == 0 ) {
		PRINT_LOG(5, "End of data");
		return E_TGC_END;
	} 
	
	if (nread < 0) {
		PRINT_LOG(3, "Error reading from socket");
		return E_TGC_READ;
	}

	//poor man's encryption
	if (key)
		for(i=0; i<nread; buf[i++]^=key);

	if ( (nwritten=write(tx_sd, buf, nread)) < nread ) {
		PRINT_LOG(3, "Error writing to socket");
		return E_TGC_WRITE;
	}

	return 0;
}

/*-----------------------------------------------------------------------------
 * Pumping from one socket to another
------------------------------------------------------------------------------*/
int tgc_pump(int sdi, int sdx, unsigned char *buf, unsigned char key)
{
	fd_set	rfds, reads;
	int	rc=0;


	if (sdi<0 || sdx<0 || !buf)
		return E_TGC_IE;

	FD_ZERO(&rfds);
	FD_SET(sdi, &rfds);
	FD_SET(sdx, &rfds);

	while (sdi>=0 && sdx>=0) {
		reads = rfds; 
		if ( select(FD_SETSIZE, &reads, NULL, NULL, NULL) < 0 ) 
			continue;

		if (FD_ISSET(sdi, &reads)) {
			if ( (rc=tgc_rxtx(sdi, sdx, buf, key)) < 0 ) {
				close_connection(&sdi);
				close_connection(&sdx);
				return rc;
			}
		}

		if (FD_ISSET(sdx, &reads)) {
			if ( (rc=tgc_rxtx(sdx, sdi, buf, key)) < 0 ) {
				close_connection(&sdi);
				close_connection(&sdx);
				return rc;
			}
		}
	}
	return rc;
}

/*-----------------------------------------------------------------------------
 * Check Filter
------------------------------------------------------------------------------*/
int tgc_check_filter(TGC *tgc, const char *ip)
{
	int	ec=0;
	char 	cmd[MAX_PATH + 1];

	if (!tgc || !ip) 
		return 0;
	if (!strlen(tgc->filter))
		return 1;
	// run the filter
	PRINT_LOG(3, "Received a client from %s", ip);
	snprintf(cmd, MAX_PATH, "%s %s", tgc->filter, ip);
	ec = system(cmd);
	if (ec == -1) {
		PRINT_LOG(1, "filter %s failed", cmd);
		return 0;
	} else {
		ec >>= 8;
		if (ec) { //filter return false
			PRINT_LOG(3, "filter %s rejected", cmd);
			return 0; //filter return false
		}
		// we are here, which means filter returned true
		PRINT_LOG(3, "filter %s permited", cmd);
	}
	return 1;
}

/*-----------------------------------------------------------------------------
 * LL node
------------------------------------------------------------------------------*/
int tgc_ll(TGC *tgc)
{
	fd_set		rfds, reads;
	struct	sockaddr_in addr;
	socklen_t 	in_addrlen = sizeof(addr);
	int		rc=0, sdi, sdx, sd;
	socket_pair_list	*conn, *prev_conn;
	char		cmd;
	struct in_addr	cc_addr = { 0 } ;
	char		ip[16];
	int		close_control = 0;


	if (!tgc)
		return E_TGC_IE;

	if ( (tgc->sdx_accept=open_server_socket(tgc->node.ll.port)) < 0 ) {
		PRINT_LOG(1, "Can't bind to port %d", tgc->node.ll.port);
		return E_TGC_NOCANDO;
	}

	if ( (tgc->sdi_accept=open_server_socket(tgc->node.ll.ll_port)) < 0 ) {
		PRINT_LOG(1, "Can't bind to port %d", tgc->node.ll.ll_port);
		return E_TGC_NOCANDO;
	}

	PRINT_LOG(3, "waiting for client on port %d ...", tgc->node.ll.port);
	PRINT_LOG(3, "waiting for CC on %d!", tgc->node.ll.ll_port);
	
	FD_ZERO(&rfds);
	FD_SET(tgc->sdi_accept, &rfds);
	FD_SET(tgc->sdx_accept, &rfds);

	tgc->node.ll.control_sd = -1;

	while(1) {
		reads = rfds;
		if ( select(FD_SETSIZE, &reads, NULL, NULL, NULL) < 0 ) 
			continue;

		if (FD_ISSET(tgc->sdx_accept, &reads)) { // incoming client connection
			sdx = accept_connection(tgc->sdx_accept, (struct sockaddr_in *)&addr, 
				       		(socklen_t *)&in_addrlen);

			if (sdx==-1) {
				if (errno==EINTR)
					continue;
				PRINT_LOG(1, "Error accepting new client connection");
				continue; //break; // exit
			}

			if (tgc->node.ll.control_sd<0) { 
				//client came in while there is no control connection, so we close it
				PRINT_LOG(3, "rejecting client connection before control connection received ");
				close_connection(&sdx);
				continue;
			}

			strncpy(ip, inet_ntoa(addr.sin_addr), 16);
			PRINT_LOG(3, "client (%s) connected on %d!", ip, tgc->node.ll.ll_port);
			// run the filter
			if (!tgc_check_filter(tgc, ip)) {
				close_connection(&sdx);
				continue;
			}
			PRINT_LOG(3, "Ask CC for a connection for the new client");

			if (tgc_send(tgc->node.ll.control_sd, TGC_COMM_CCC)>=0) {
				if (tgc_add_queue( &(tgc->node.ll.socketq), sdx)<0) {
					PRINT_LOG(1, "Error adding socket to queue!");
					close_connection(&sdx);
					return E_TGC_NOCANDO;
				}
				PRINT_LOG(5, "client connection added to the queue");
		 	} else 
				close_connection(&sdx); // couldn't ask CC for a new connection, close the client
		} 
		
		if (FD_ISSET(tgc->sdi_accept, &reads)) { // from CC
			sdi = accept_connection(tgc->sdi_accept, (struct sockaddr_in *)&addr, 
				       (socklen_t *)&in_addrlen);

			if (sdi==-1) {
				if (errno==EINTR)
					continue;
				PRINT_LOG(1, "Error accepting new CC connection");
				break; // exit
			}

			strncpy(ip, inet_ntoa(addr.sin_addr), 16);
			PRINT_LOG(3, "CC connected from %s", ip);
			// run the filter
			if (!tgc_check_filter(tgc, ip)) {
				close_connection(&sdi);
                                continue;
			}

			if (tgc->node.ll.control_sd<0) { 
				// it's the control connection
				tgc->node.ll.control_sd = sdi;
				cc_addr.s_addr = addr.sin_addr.s_addr;
				FD_SET(sdi, &rfds);
				PRINT_LOG(4, "Control connection established");
			} else { // it's a normal CC connection
				if (addr.sin_addr.s_addr!=cc_addr.s_addr || !tgc->node.ll.socketq) {
					// if this CC's address is different from control_sd's OR
					// if there are no client connections waiting in the queue
					// we reject the CC
					close_connection(&sdi);
					PRINT_LOG(1, "Suspicious CC rejected");
				} else { //everything seems good, lets do our thing!
					sdx = tgc_remove_queue(&(tgc->node.ll.socketq));
					if (sdx>=0) {
						if (tgc->method==TGC_METHOD_SELECT) {
							if (!tgc_add_list( &(tgc->pairs), sdi, sdx)) {
								FD_SET(sdi, &rfds);
								FD_SET(sdx, &rfds);
							} 
						} else {
#ifdef HAVE_FORK
							// FORK
							if (fork()==0) { // child
								close_connection(&(tgc->sdi_accept));
								close_connection(&(tgc->sdx_accept));
								close_connection(&(tgc->node.ll.control_sd));
								tgc_pump(sdi, sdx, tgc->buf, tgc->key);
								break; //exit(0);
							} else { // parent
								close_connection(&sdi);
								close_connection(&sdx);
							}
#endif
						}
					}
				}
			}
		} 
		
		if (tgc->node.ll.control_sd>=0 && FD_ISSET(tgc->node.ll.control_sd, &reads)) {
			close_control = 0;
			if (tgc_read(tgc->node.ll.control_sd, &cmd) == E_TGC_OK) {
				switch (cmd) {
					case TGC_COMM_PING:
						PRINT_LOG(2, "Received a ping from CC");
						if (tgc_send(tgc->node.ll.control_sd, TGC_COMM_PING)<0) {
							PRINT_LOG(1, "Internal Error: can't write to control connection");
							FD_CLR(tgc->node.ll.control_sd, &rfds);
							close_connection(&(tgc->node.ll.control_sd));
							tgc->node.ll.control_sd = -1;
							continue;
						}
						break;
					case TGC_COMM_CLOSE:
						PRINT_LOG(3, "CC wants us to close the lingering client connection!");
						if ((sd=tgc_remove_queue(&(tgc->node.ll.socketq)))>=0)
							close_connection(&sd);
						break;
					default:
						PRINT_LOG(1, "Internal Error: Uknown code (0x%x) recieved from CC!", cmd);
						close_control = 1;
				}
			} else {
				close_control = 1;
			}

			if (close_control) {
				PRINT_LOG(1, "closing control connection");
				FD_CLR(tgc->node.ll.control_sd, &rfds);
				close_connection(&(tgc->node.ll.control_sd));
				tgc->node.ll.control_sd = -1;
				close_control = 0;
				continue;
			}
		}

		// pump the data for each and every connection pairs
		if (tgc->method==TGC_METHOD_SELECT) {
			conn = tgc->pairs; 
			while (conn) {
				if (FD_ISSET(conn->sdi , &reads)) { // from CC?
					if ( (rc=tgc_rxtx(conn->sdi, conn->sdx, tgc->buf, tgc->key)) < 0 ) {
						FD_CLR(conn->sdi, &rfds);
						FD_CLR(conn->sdx, &rfds);
						close_connection(&(conn->sdi));
						close_connection(&(conn->sdx));
						prev_conn = conn;
						conn = conn->next;
						tgc_remove_list( &(tgc->pairs), prev_conn);
						continue;
					}
				}

				if (FD_ISSET(conn->sdx, &reads)) { // from client?
					if ( (rc=tgc_rxtx(conn->sdx, conn->sdi, tgc->buf, tgc->key)) < 0 ) {
						FD_CLR(conn->sdi, &rfds);
						FD_CLR(conn->sdx, &rfds);
						close_connection(&(conn->sdi));
						close_connection(&(conn->sdx));
						prev_conn = conn;
						conn = conn->next;
						tgc_remove_list( &(tgc->pairs), prev_conn);
						continue;
					}
				}
				conn=conn->next;
			}
		}
	}
	
	return 0;
}


/*-----------------------------------------------------------------------------
* Become a ConnectConnect node
*------------------------------------------------------------------------------*/
int tgc_cc(TGC *tgc)
{
	int     sdi, sdx, rc=0;
	struct timeval tv;
	time_t	last_time;
	fd_set  rfds, reads;
	char	cmd;
	socket_pair_list	*conn, *prev_conn;
	int 	close_control=0;


	if (!tgc)
		return E_TGC_IE;

	tgc->node.cc.control_sd = -2;
	last_time = time(NULL);

	FD_ZERO(&rfds);

	while (1) {

		if (tgc->node.cc.control_sd == -1) // is it a retry?
			sleep(tgc->node.cc.interval);
		
		// connect to LL
		if ( (tgc->node.cc.control_sd=connect_server(LL_HOST, CC_LL_PORT))<0 ) {
			PRINT_LOG(2, "failed connecting to %s:%d", LL_HOST, CC_LL_PORT);
			tgc->node.cc.control_sd = -1;
			continue;
		}
		PRINT_LOG(3, "Control connection to CC on %s:%d established", LL_HOST, CC_LL_PORT);
	
		FD_SET(tgc->node.cc.control_sd, &rfds);
			
		while(tgc->node.cc.control_sd>=0) {
			tv.tv_sec = tgc->node.cc.interval;
			tv.tv_usec = 0;
			reads = rfds;
			if ( (rc=select(FD_SETSIZE, &reads, NULL, NULL, &tv))<0 ) {
				PRINT_LOG(1, "Error on select");
				continue;
			}
			
			if (rc==0 || (last_time+tgc->node.cc.interval)<time(NULL)) { 
				//either timed out or it's time to send ping again
				PRINT_LOG(2, "Sending ping to LL");
				if (tgc_send(tgc->node.cc.control_sd, TGC_COMM_PING) < 0) {
					PRINT_LOG(1, "Ping failed, closing the control connection!");
					FD_CLR(tgc->node.ll.control_sd, &rfds);
					close_connection(&(tgc->node.cc.control_sd));
					tgc->node.cc.control_sd = -1;
					break;
				}
				last_time = time(NULL);
			}

			if (FD_ISSET(tgc->node.cc.control_sd, &reads)) {
				close_control = 0;
				if (tgc_read(tgc->node.cc.control_sd, &cmd) == E_TGC_OK) {
					switch (cmd) {
						case TGC_COMM_PING:
							PRINT_LOG(2, "Recieved a ping from LL");
							// Meh, we just ignore the ping!
							break;
						case TGC_COMM_CCC:
							PRINT_LOG(3, "LL wants us a new connection");
							// first try to connect to the server!
							if ( (sdx=connect_server(CC_SERVER, CC_SERVER_PORT))<0 ) {
								PRINT_LOG(2, "failed connecting to server to %s:%d", CC_SERVER, CC_SERVER_PORT);
								tgc_send(tgc->node.cc.control_sd, TGC_COMM_CLOSE);
								continue; 
							}

							// now connect to LL
							if ( (sdi=connect_server(LL_HOST, CC_LL_PORT))<0 ) {
								PRINT_LOG(2, "failed connecting to LL %s:%d", LL_HOST, CC_LL_PORT);
								close_connection(&sdx);
								sdx = -1;
								tgc_send(tgc->node.cc.control_sd, TGC_COMM_CLOSE);
								continue;
							}
							PRINT_LOG(3, "connected to server %s:%d", LL_HOST, CC_LL_PORT);
			
							if (sdx>=0 && sdi>=0) {
								if (tgc->method==TGC_METHOD_SELECT) {
									if (!tgc_add_list( &(tgc->pairs), sdi, sdx)) {
										FD_SET(sdi, &rfds);
										FD_SET(sdx, &rfds);
									} 
								} else { // FORK
#ifdef HAVE_FORK
									if (fork()==0) { // child
										close_connection(&(tgc->sdi_accept));
										close_connection(&(tgc->sdx_accept));
										close_connection(&(tgc->node.cc.control_sd));
										tgc_pump(sdi, sdx, tgc->buf, tgc->key);
										exit(0);
									} else { // parent
										close_connection(&sdi);
										close_connection(&sdx);
									}
#endif
								}
							}

							break;
						default:
							PRINT_LOG(1, "Uknown command received, closing control connection");
							close_control = 1;
					}
				} else {
					close_control = 1;
				}
						
				if (close_control) {
					PRINT_LOG(1, "Closing control connection to LL");
					FD_CLR(tgc->node.cc.control_sd, &rfds);
					close_connection(&(tgc->node.cc.control_sd));
					tgc->node.cc.control_sd = -1;
					close_control = 0;
				}
			}

			// pump the data for each and every connection pairs
			if (tgc->method==TGC_METHOD_SELECT) {
				conn = tgc->pairs; 
				while (conn) {
					
					if (FD_ISSET(conn->sdi , &reads)) { // from CC?
						if ( (rc=tgc_rxtx(conn->sdi, conn->sdx, tgc->buf, tgc->key)) < 0 ) {
							FD_CLR(conn->sdi, &rfds);
							FD_CLR(conn->sdx, &rfds);
							close_connection(&(conn->sdi));
							close_connection(&(conn->sdx));
							prev_conn = conn;
							conn = conn->next;
							tgc_remove_list( &(tgc->pairs), prev_conn);
							continue;
						}
					}

					if (FD_ISSET(conn->sdx, &reads)) { // from client?
						if ( (rc=tgc_rxtx(conn->sdx, conn->sdi, tgc->buf, tgc->key)) < 0 ) {
							FD_CLR(conn->sdi, &rfds);
							FD_CLR(conn->sdx, &rfds);
							close_connection(&(conn->sdi));
							close_connection(&(conn->sdx));
							prev_conn = conn;
							conn = conn->next;
							tgc_remove_list( &(tgc->pairs), prev_conn);
							continue;
						}
					}

					conn=conn->next;
				}
			}
		} // while(tgc->node.cc.control_sd>=0)
	} // while(1)
	
        return 0;
}

/*-----------------------------------------------------------------------------
  Become a port forwarder
------------------------------------------------------------------------------*/
int tgc_pf(TGC *tgc)
{
	struct sockaddr_in 	addr;
	socklen_t    		in_addrlen = sizeof(addr);
	fd_set 			rfds, reads;
	int			sdi, sdx, rc=0;
	socket_pair_list	*conn, *prev_conn;
	char			ip[16];


	signal(SIGCLD, SIG_DFL); // to avoid zombie process

 	if ( (tgc->sdi_accept=open_server_socket(tgc->node.pf.port)) < 0 ) {
		PRINT_LOG(1, "Can't bind to port %d", tgc->node.pf.port);
		return E_TGC_NOCANDO;
	}
	
	PRINT_LOG(3, "waiting for incomming connections on port %d ...", tgc->node.pf.port);

	FD_ZERO(&rfds);
	FD_SET(tgc->sdi_accept, &rfds);

	while (1) {

		reads = rfds;
		if ( select(FD_SETSIZE, &reads, NULL, NULL, NULL) < 0 ) 
			continue;

		if (FD_ISSET(tgc->sdi_accept, &reads)) {
		
			sdi = accept_connection(tgc->sdi_accept, (struct sockaddr_in *) &addr, 
					                     (socklen_t *)&in_addrlen);

			if (sdi==-1) {
				if (errno==EINTR)
					continue;

				PRINT_LOG(0, "Error accepting new connection!");
				close_connection(&(tgc->sdi_accept));
				break; //exit
			}

			// run the filter
			strncpy(ip, inet_ntoa(addr.sin_addr), 16);
			PRINT_LOG(3, "Received a client from %s", ip);
			if (!tgc_check_filter(tgc, ip)) {
				close_connection(&sdi);
				sdi = -1;
				continue;
			}

			if ( (sdx=connect_server(tgc->node.pf.dst_host, tgc->node.pf.dst_port))<0 ) {
				PRINT_LOG(1, "failed connecting to %s:%d", tgc->node.pf.dst_host, tgc->node.pf.dst_port);
				close_connection(&sdi);
				sdi = -1;
				continue;
			}
			PRINT_LOG(3, "connected to the server");

			if (tgc->method == TGC_METHOD_SELECT) {
				if (!tgc_add_list( &(tgc->pairs), sdi, sdx)) {
					FD_SET(sdi, &rfds);
					FD_SET(sdx, &rfds);
					PRINT_LOG(3, "Added socket pairs to the list");
					continue;
				}
			} else { //fork method
#ifdef HAVE_FORK
				if (fork()==0) { //child
					close_connection(&(tgc->sdi_accept));
					tgc_pump(sdi, sdx, tgc->buf, tgc->key);
					break;
				} else {// parent
					close_connection(&sdi);
					close_connection(&sdx);
					PRINT_LOG(3, "waiting for incomming connections on port %d again...", tgc->node.pf.port);
				}
#endif
			}
		}

		// pump the data for each and every connection pairs
		if (tgc->method==TGC_METHOD_SELECT) {
			conn = tgc->pairs;
			while (conn) {
				if (FD_ISSET(conn->sdi , &reads)) { // from client
					if ( (rc=tgc_rxtx(conn->sdi, conn->sdx, tgc->buf, tgc->key)) < 0 ) {
						PRINT_LOG(3, "Error reading socket, closing pair");
						FD_CLR(conn->sdi, &rfds);
						FD_CLR(conn->sdx, &rfds);
						close_connection(&(conn->sdi));
						close_connection(&(conn->sdx));
						prev_conn = conn;
						conn = conn->next;
						tgc_remove_list( &(tgc->pairs), prev_conn);
						continue;
					}
				}

				if (FD_ISSET(conn->sdx, &reads)) { // from server
					if ( (rc=tgc_rxtx(conn->sdx, conn->sdi, tgc->buf, tgc->key)) < 0 ) {
						PRINT_LOG(3, "Error reading socket, closing pair.");
						FD_CLR(conn->sdi, &rfds);
						FD_CLR(conn->sdx, &rfds);
						close_connection(&(conn->sdi));
						close_connection(&(conn->sdx));
						prev_conn = conn;
						conn=conn->next;
						tgc_remove_list( &(tgc->pairs), prev_conn);
						continue;
					}
				}
				conn=conn->next;
			}
		}
	}
	return E_TGC_NOCANDO;
}


/*-----------------------------------------------------------------------------
 * Decide what to do!
------------------------------------------------------------------------------*/
int tgc_run(TGC *tgc) 
{
	int rc;

	switch (tgc->type) {
		case CCNODE:
			PRINT_LOG(3, "Become a CC node");
			rc = tgc_cc(tgc);
			break;
		case LLNODE:
			PRINT_LOG(3, "Become a LL node");
			rc = tgc_ll(tgc);
			break;
		case FNODE:
			PRINT_LOG(3, "Become a PF node");
			rc = tgc_pf(tgc);
			break;
		default:
			PRINT_LOG(1, "Invalid node value");
			rc = E_TGC_NOCANDO;
	}

	return rc;
}


