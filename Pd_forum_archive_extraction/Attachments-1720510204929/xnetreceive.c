/* --------------------------  xnetreceive ------------------------------------ */
/*                                                                              */
/* xnetreceive :: eXtended netreceive.                                          */
/* Written by Jorge Cardoso <jorgecardoso@ieee.org>                             */
/* Inspired by [netreceive] and [maxlib_netserver]								*/
/*                                                                              */
/*                                                                              */
/* This program is free software; you can redistribute it and/or                */
/* modify it under the terms of the GNU General Public License                  */
/* as published by the Free Software Foundation; either version 2               */
/* of the License, or (at your option) any later version.                       */
/*                                                                              */
/* This program is distributed in the hope that it will be useful,              */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of               */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                */
/* GNU General Public License for more details.                                 */
/*                                                                              */
/* You should have received a copy of the GNU General Public License            */
/* along with this program; if not, write to the Free Software                  */
/* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.  */
/*                                                                              */
/* Based on PureData by Miller Puckette and others.  							*/
/*                                                                              */
/* ---------------------------------------------------------------------------- */

#include "m_pd.h"
#include "s_stuff.h"
#include "m_imp.h"

#include <sys/types.h>
#include <stdarg.h>
#include <signal.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h> //mp
#include <pthread.h>

//#ifdef UNIX
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h> //mp
#include <netdb.h>
#include <stdio.h>
#define SOCKET_ERROR -1
/*#else
#include <io.h>
#include <fcntl.h>
#include <winsock.h>
#endif*/


#define MAX_CONNECT  32		/* maximum number of connections */
#define INBUFSIZE 4096

static t_binbuf *inbinbuf;

static t_class *xnetreceive_class;

typedef void (*t_xsocketnotifier) (void *x);
typedef void (*t_xsocketreceivefn) (void *x, t_binbuf * b);

typedef struct _xnetreceive {
    t_object x_obj;
    t_outlet *x_msgout;
    t_outlet *x_connectout;
    t_outlet *x_ipout;
    t_outlet *x_portout;
	char *ip;
	int port;

	t_symbol *x_host[MAX_CONNECT];
	t_int    x_fd[MAX_CONNECT];
	t_int	 x_port[MAX_CONNECT];
	t_int    x_nconnections;
	t_int    x_sock_fd;
    int x_connectsocket;
    int x_udp;
} t_xnetreceive;

struct _xsocketreceiver {
    char *sr_inbuf;
    int sr_inhead;
    int sr_intail;
    void *sr_owner;
    int sr_udp;
    t_xsocketnotifier sr_notifier;
    t_xsocketreceivefn sr_socketreceivefn;
};

#define t_xsocketreceiver struct _xsocketreceiver

static t_xsocketreceiver *sys_xsocketreceiver;

t_xsocketreceiver *xsocketreceiver_new(void *owner,
				       t_xsocketnotifier notifier,
				       t_xsocketreceivefn socketreceivefn,
				       int udp)
{
    t_xsocketreceiver *x = (t_xsocketreceiver *) getbytes(sizeof(*x));
    x->sr_inhead = x->sr_intail = 0;
    x->sr_owner = owner;
    x->sr_notifier = notifier;
    x->sr_socketreceivefn = socketreceivefn;
    x->sr_udp = udp;
    if (!(x->sr_inbuf = malloc(INBUFSIZE)))
	bug("t_socketreceiver");;
    return (x);
}

void xsocketreceiver_free(t_xsocketreceiver * x)
{
    free(x->sr_inbuf);
    freebytes(x, sizeof(*x));
}

    /*
     * this is in a separately called subroutine so that the buffer isn't
     * sitting on the stack while the messages are getting passed.
     */
static int xsocketreceiver_doread(t_xsocketreceiver * x)
{
    char messbuf[INBUFSIZE], *bp = messbuf;
    int indx;
    int inhead = x->sr_inhead;
    int intail = x->sr_intail;
    char *inbuf = x->sr_inbuf;

    if (intail == inhead) {
		return (0);
	}
    for (indx = intail; indx != inhead; indx = (indx + 1) & (INBUFSIZE - 1)) {
		/*
		 * if we hit a semi that isn't preceeded by a \, it's a message
		 * boundary.  LATER we should deal with the possibility that the
		 * preceeding \ might itself be escaped!
		 */
		char c = *bp++ = inbuf[indx];
		if (c == ';' && (!indx || inbuf[indx - 1] != '\\')) {
	    	intail = (indx + 1) & (INBUFSIZE - 1);
	    	binbuf_text(inbinbuf, messbuf, bp - messbuf);

			/*
			 * if (sys_debuglevel & DEBUG_MESSDOWN) { write(2, messbuf, bp
			 * - messbuf); write(2, "\n", 1); }
			 */
		    x->sr_inhead = inhead;
	    	x->sr_intail = intail;
	    	return (1);
		}
    }
    return (0);
}

static void x_socketreceiver_getudp(t_xsocketreceiver * x, int fd)
{
	struct sockaddr_in incomer_address;
    char buf[INBUFSIZE + 1];
    int pcli_len;
	int ret;
	t_xnetreceive *y;

	pcli_len = sizeof(incomer_address);
	ret = recvfrom(fd, buf, INBUFSIZE, 0, (struct sockaddr *)&incomer_address, &pcli_len);


    y = (t_xnetreceive *)x->sr_owner;
	y->ip = inet_ntoa(incomer_address.sin_addr);
	y->port = ntohs(incomer_address.sin_port);

    if (ret < 0) {
		sys_sockerror("recvfrom");
		sys_rmpollfn(fd);
		sys_closesocket(fd);
    } else if (ret > 0) {
		buf[ret] = 0;
#if 0
	post("%s", buf);
#endif
		if (buf[ret - 1] != '\n') {
#if 0
	    	buf[ret] = 0;
	    	error("dropped bad buffer %s\n", buf);
#endif
		} else {
	    	char *semi = strchr(buf, ';');
	    	if (semi) {
				*semi = 0;
			}
	    	binbuf_text(inbinbuf, buf, strlen(buf));
	    	outlet_setstacklim();
	    	if (x->sr_socketreceivefn) {
				(*x->sr_socketreceivefn) (x->sr_owner, inbinbuf);
			} else {
				bug("socketreceiver_getudp");
			}
		}
    }
}



void x_socketreceiver_read(t_xsocketreceiver * x, int fd)
{
	struct sockaddr_in incomer_address;
	int pcli_len;
	t_xnetreceive *y;

	y = (t_xnetreceive *)x->sr_owner;
	y->x_sock_fd = fd;

    if (x->sr_udp) {		/* UDP ("datagram") socket protocol */

		x_socketreceiver_getudp(x, fd);

    } else {				/* TCP ("streaming") socket protocol */

		char *semi;
		int readto = (x->sr_inhead >= x->sr_intail ? INBUFSIZE : x->sr_intail - 1);
		int ret;

		/*
		 * the input buffer might be full.  If so, drop the whole thing
		 */
		if (readto == x->sr_inhead) {
	    	fprintf(stderr, "pd: dropped message from gui\n");
	    	x->sr_inhead = x->sr_intail = 0;
	    	readto = INBUFSIZE;
		} else {

			pcli_len = sizeof(incomer_address);
			ret = recvfrom(fd, x->sr_inbuf + x->sr_inhead, readto - x->sr_inhead, 0, (struct sockaddr *)&incomer_address, &pcli_len);

			y->ip = inet_ntoa(incomer_address.sin_addr);
			y->port = ntohs(incomer_address.sin_port);

	    	if (ret < 0) {
				sys_sockerror("recv");
				if (x == sys_xsocketreceiver) {
		    		sys_bail(1);
				} else {

		    		if (x->sr_notifier) {
						(*x->sr_notifier) (x->sr_owner);
					}
		    		sys_rmpollfn(fd);
		    		sys_closesocket(fd);
				}

	    	} else if (ret == 0) {
				if (x == sys_xsocketreceiver) {
		    		fprintf(stderr, "pd: exiting\n");
		    		sys_bail(0);
				} else {
		    		post("EOF on socket %d\n", fd);
		    		if (x->sr_notifier) {
						(*x->sr_notifier) (x->sr_owner);
					}
		    		sys_rmpollfn(fd);
		    		sys_closesocket(fd);
				}
	    	} else {
				x->sr_inhead += ret;
				if (x->sr_inhead >= INBUFSIZE) {
		    		x->sr_inhead = 0;
				}
				while (xsocketreceiver_doread(x)) {
					outlet_setstacklim();
		    		if (x->sr_socketreceivefn) {
						(*x->sr_socketreceivefn) (x->sr_owner, inbinbuf);
					} else {
						binbuf_eval(inbinbuf, 0, 0, 0);
					}
				}
	    	}
		}
    }
}

static void xnetreceive_notify(t_xnetreceive * x)
{
	int i, k;
	if (!x->x_udp) {
			/* remove connection from list */
		for(i = 0; i < x->x_nconnections; i++) {
			if(x->x_fd[i] == x->x_sock_fd) {
				x->x_nconnections--;
				post("netserver: \"%s\" removed from list of clients", x->x_host[i]->s_name);
				x->x_host[i] = NULL;	/* delete entry */
				x->x_fd[i] = -1;
				/* rearrange list now: move entries to close the gap */
				for(k = i; k < x->x_nconnections; k++) {
					x->x_host[k] = x->x_host[k + 1];
					x->x_fd[k] = x->x_fd[k + 1];
				}
			}
		}
    	outlet_float(x->x_connectout, x->x_nconnections);
	}

//	post("xnetreceive_notify\n");
   // outlet_float(x->x_connectout, --x->x_nconnections);
}

static void xnetreceive_doit(void *z, t_binbuf * b)
{
    t_atom messbuf[1024];
    t_xnetreceive *x = (t_xnetreceive *) z;
    int msg, natom = binbuf_getnatom(b);
    t_atom *at = binbuf_getvec(b);
	int i;


	if (x->x_udp) {
		outlet_float(x->x_portout, x->port);
		if ( x->ip != NULL) {
		    outlet_symbol(x->x_ipout, gensym(x->ip));
		}
	} else {
		/* output clients IP and socket no. */
		for(i = 0; i < x->x_nconnections; i++)	/* search for corresponding IP */
		{
			if(x->x_fd[i] == x->x_sock_fd)
			{
				outlet_float(x->x_portout, x->x_port[i]);
				outlet_symbol(x->x_ipout, x->x_host[i]);

				break;
			}
		}

	}


    for (msg = 0; msg < natom;) {
		int emsg;
		for (emsg = msg; emsg < natom && at[emsg].a_type != A_COMMA && at[emsg].a_type != A_SEMI; emsg++);
		if (emsg > msg) {
		    int j;
		    for (j = msg; j < emsg; j++) {
				if (at[j].a_type == A_DOLLAR || at[j].a_type == A_DOLLSYM) {
				    pd_error(x, "netreceive: got dollar sign in message");
				    goto nodice;
				}
			}
	    	if (at[msg].a_type == A_FLOAT) {
				if (emsg > msg + 1) {
				    outlet_list(x->x_msgout, 0, emsg - msg, at + msg);
				}
			 	else {
		    		outlet_float(x->x_msgout, at[msg].a_w.w_float);
				}
	    	} else if (at[msg].a_type == A_SYMBOL) {
				outlet_anything(x->x_msgout, at[msg].a_w.w_symbol,emsg - msg - 1, at + msg + 1);
			}
		}
nodice:
	msg = emsg + 1;
    }
}

static void xnetreceive_connectpoll(t_xnetreceive * x)
{
	struct sockaddr_in incomer_address;
	int sockaddrl = (int) sizeof( struct sockaddr );
    int fd = accept(x->x_connectsocket, (struct sockaddr*)&incomer_address, &sockaddrl);

    if (fd < 0) {
		post("netreceive: accept failed");
	} else {
		t_xsocketreceiver *y = xsocketreceiver_new((void *) x,
						   (t_xsocketnotifier)
						   xnetreceive_notify,
						   (x->
						    x_msgout ?
						    xnetreceive_doit : 0),
						   0);
		sys_addpollfn(fd, (t_fdpollfn) x_socketreceiver_read, y);
		x->x_nconnections++;
		x->x_host[x->x_nconnections - 1] = gensym(inet_ntoa(incomer_address.sin_addr));
		x->x_port[x->x_nconnections - 1] = ntohs(incomer_address.sin_port);
		x->x_fd[x->x_nconnections - 1] = fd;
		outlet_float(x->x_connectout, x->x_nconnections);
    }
}

static void *xnetreceive_new(t_symbol * compatflag, t_floatarg fportno, t_floatarg udpflag)
{
    t_xnetreceive *x;
    struct sockaddr_in server;
    int sockfd, portno = fportno, udp = (udpflag != 0);
    int old = !strcmp(compatflag->s_name, "old");
    int intarg;
    int i;

    /*
     * create a socket
     */
    sockfd = socket(AF_INET, (udp ? SOCK_DGRAM : SOCK_STREAM), 0);


#if 0
    fprintf(stderr, "receive socket %d\n", sockfd);
#endif
    if (sockfd < 0) {
		sys_sockerror("socket");
		return (0);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

#if 1
    /*
     * ask OS to allow another Pd to repoen this port after we close it.
     */
    intarg = 1;
    if (setsockopt(sockfd, SOL_SOCKET,
		   SO_REUSEADDR, (const char *) &intarg,
		   sizeof(intarg)) < 0)
	post("setsockopt (SO_REUSEADDR) failed\n");
#endif
#if 0
    intarg = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &intarg, sizeof(intarg)) < 0) {
		post("setsockopt (SO_RCVBUF) failed\n");
	}
#endif

    /*
     * Stream (TCP) sockets are set NODELAY
     */
    if (!udp) {
		intarg = 1;

		if (setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY,
		       (const char *) &intarg, sizeof(intarg)) < 0) {
		    post("setsockopt (TCP_NODELAY) failed\n");
		}
    }

    /*
     * assign server port number
     */
    server.sin_port = htons((u_short) portno);

    /*
     * name the socket
     */
    if (bind(sockfd, (struct sockaddr *) &server, sizeof(server)) < 0) {
		sys_sockerror("bind");
		sys_closesocket(sockfd);
		return (0);
    }

    x = (t_xnetreceive *) pd_new(xnetreceive_class);

    if (old) {
		/*
		 * old style, nonsecure version
		 */
		x->x_msgout = 0;
    } else {
		x->x_msgout = outlet_new(&x->x_obj, &s_anything);
	}



    if (udp) {			/* datagram protocol */
		t_xsocketreceiver *y = xsocketreceiver_new((void *) x,
						   (t_xsocketnotifier) xnetreceive_notify,
						   (x->x_msgout ? xnetreceive_doit : 0),  1);
		sys_addpollfn(sockfd, (t_fdpollfn) x_socketreceiver_read, y);


		x->x_connectout = 0;
		x->x_ipout = outlet_new(&x->x_obj, &s_float);
		x->x_portout = outlet_new(&x->x_obj, &s_float);

    } else {			/* streaming protocol */
		if (listen(sockfd, 5) < 0) {
		    sys_sockerror("listen");
		    sys_closesocket(sockfd);
		    sockfd = -1;
		} else {
		    sys_addpollfn(sockfd, (t_fdpollfn) xnetreceive_connectpoll, x);

			x->x_ipout = outlet_new(&x->x_obj, &s_float);
			x->x_portout = outlet_new(&x->x_obj, &s_float);
		    x->x_connectout = outlet_new(&x->x_obj, &s_float);
		}
    }
    inbinbuf = binbuf_new();
    x->x_connectsocket = sockfd;
    x->x_nconnections = 0;
    x->x_udp = udp;

    x->x_nconnections = 0;
	for(i = 0; i < MAX_CONNECT; i++)x->x_fd[i] = -1;


    return (x);
}

static void xnetreceive_free(t_xnetreceive * x)
{
    /*
     * LATER make me clean up open connections
     */

    if (x->x_connectsocket >= 0) {
		sys_rmpollfn(x->x_connectsocket);
		sys_closesocket(x->x_connectsocket);
    }
    binbuf_free(inbinbuf);
}

void xnetreceive_setup(void)
{
    xnetreceive_class = class_new(gensym("xnetreceive"),
				  (t_newmethod) xnetreceive_new,
				  (t_method) xnetreceive_free,
				  sizeof(t_xnetreceive), CLASS_NOINLET,
				  A_DEFFLOAT, A_DEFFLOAT, A_DEFSYM, 0);
}

