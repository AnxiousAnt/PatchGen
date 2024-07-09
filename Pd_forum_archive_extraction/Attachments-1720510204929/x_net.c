/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* network */

#include "m_imp.h"

#include <sys/types.h>
#include <string.h>
#ifdef UNIX
#include <sys/errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdio.h>
#define SOCKET_ERROR -1
#else
#include <winsock.h>
#define SOL_TCP IPPROTO_TCP
#define MSG_NOSIGNAL 0
#endif

static t_class *netsend_class;

typedef struct _netsend
{
    t_object x_obj;
    int x_fd;
    int x_protocol;
} t_netsend;

static void *netsend_new(t_floatarg udpflag)
{
    t_netsend *x = (t_netsend *)pd_new(netsend_class);
    outlet_new(&x->x_obj, &s_float);
    x->x_fd = -1;
    x->x_protocol = (udpflag != 0 ? SOCK_DGRAM : SOCK_STREAM);
    return (x);
}

static void netsend_disconnect(t_netsend *x)
{
    if (x->x_fd >= 0)
    {
    	sys_closesocket(x->x_fd);
    	x->x_fd = -1;
    	outlet_float(x->x_obj.ob_outlet, 0);
    }
}


static void netsend_connect(t_netsend *x, t_symbol *hostname,
    t_floatarg fportno)
{
    struct sockaddr_in server;
    struct hostent *hp;
    int sockfd;
    int portno = fportno;
    int sockopt;

    if (x->x_fd >= 0)
    {
	netsend_disconnect(x);
#if 0
    	error("netsend_connect: already connected");
    	return;
#endif
    }
    
    /* create a socket */
    sockfd = socket(AF_INET, x->x_protocol, 0);
#if 0
    fprintf(stderr, "send socket %d\n", sockfd);
#endif
    if (sockfd < 0)
    {
    	sys_sockerror("socket");
    	return;
    }

    /* At least under NT this improves responsiveness of TCP */
    
    sockopt = 1;
    if (setsockopt(sockfd, SOL_TCP, TCP_NODELAY, (const char*) &sockopt, sizeof(int)) < 0)
    	post("setsockopt NODELAY failed\n");
    else
	 post("TCP_NODELAY set");

    /* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname->s_name);
    if (hp == 0)
    {
	post("bad host?\n");
	return;
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

    /* assign client port number */
    server.sin_port = htons((u_short)portno);

    post("connecting to port %d", portno);
	/* try to connect.  LATER make a separate thread to do this
	because it might block */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
    	sys_sockerror("connecting stream socket");
    	sys_closesocket(sockfd);
    	return;
    }
    x->x_fd = sockfd;
    outlet_float(x->x_obj.ob_outlet, 1);
}

static void netsend_send(t_netsend *x, t_symbol *s, int argc, t_atom *argv)
{
    if (x->x_fd >= 0)
    {
	t_binbuf *b = binbuf_new();
	char *buf, *bp;
	int length, sent;
	t_atom at;
	binbuf_add(b, argc, argv);
	SETSEMI(&at);
	binbuf_add(b, 1, &at);
	binbuf_gettext(b, &buf, &length);
	for (bp = buf, sent = 0; sent < length;)
	{
	    static double lastwarntime;
	    static double pleasewarn;
	    int late,res;
	    double timeafter,timebefore = sys_getrealtime();
    	    res = send(x->x_fd, buf, length-sent, MSG_NOSIGNAL);
    	    timeafter = sys_getrealtime();
    	    late = (timeafter - timebefore > 0.005);
	    
    	    if (late || pleasewarn)
    	    {
    	    	if (timeafter > lastwarntime + 2)
    	    	{
    	    	     post("netsend blocked %d msec",
    	    	     	(int)(1000 * ((timeafter - timebefore) + pleasewarn)));
    	    	     pleasewarn = 0;
    	    	     lastwarntime = timeafter;
    	    	}
    	    	else if (late) pleasewarn += timeafter - timebefore;
    	    }
    	    if (res <= 0)
    	    {
    		sys_sockerror("netsend");
    		netsend_disconnect(x);
    		break;
    	    }
    	    else
    	    {
    		sent += res;
    		bp += res;
    	    }
	}
	t_freebytes(buf, length);
	binbuf_free(b);
    }
    else error("netsend: not connected");
}

static void netsend_free(t_netsend *x)
{
    netsend_disconnect(x);
}

static void netsend_setup(void)
{
    netsend_class = class_new(gensym("netsend"), (t_newmethod)netsend_new,
    	(t_method)netsend_free,
    	sizeof(t_netsend), 0, A_DEFFLOAT, 0);
    class_addmethod(netsend_class, (t_method)netsend_connect,
    	gensym("connect"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(netsend_class, (t_method)netsend_disconnect,
    	gensym("disconnect"), 0);
    class_addmethod(netsend_class, (t_method)netsend_send, gensym("send"),
    	A_GIMME, 0);
}

/* ----------------------------- netreceive ------------------------- */

static t_class *netreceive_class;


typedef struct _fdlist {
     int fd;
     struct _fdlist* next;
} t_fdlist;


t_fdlist* fdlist_add(t_fdlist* fdl,int fd)
{
     t_fdlist* nfd = (t_fdlist*) getbytes(sizeof(t_fdlist));
     nfd->next = fdl;
     nfd->fd = fd;
     return nfd;
}

t_fdlist* fdlist_remove(t_fdlist* fdl)
{
     t_fdlist* ret = fdl->next;
     freebytes(fdl,sizeof(t_fdlist));
     return ret;
}	 


typedef struct _netreceive
{
    t_object x_obj;
    int x_connectsocket;
    int x_nconnections;
    int x_udp;
     t_fdlist* x_fdlist;
} t_netreceive;

static void netreceive_notify(t_netreceive *x)
{
    outlet_float(x->x_obj.ob_outlet, --x->x_nconnections);
}

static void netreceive_connectpoll(t_netreceive *x)
{
    int fd = accept(x->x_connectsocket, 0, 0);
    if (fd < 0) post("netreceive: accept failed");
    else
    {
    	t_socketreceiver *y = socketreceiver_new((void *)x, 
    	    (t_socketnotifier)netreceive_notify);
    	sys_addpollfn(fd, (t_fdpollfn)socketreceiver_read, y);

	x->x_fdlist = fdlist_add(x->x_fdlist,fd);	     
    	outlet_float(x->x_obj.ob_outlet, ++x->x_nconnections);
    }
}

static void *netreceive_new(t_floatarg fportno, t_floatarg udpflag)
{
    t_netreceive *x;
    struct sockaddr_in server;
    int sockfd, portno = fportno, udp = (udpflag != 0);
    int i;
    int sockopt;

    	/* create a socket */
    sockfd = socket(AF_INET, (udp ? SOCK_DGRAM : SOCK_STREAM), 0);
#if 0
    fprintf(stderr, "receive socket %d\n", sockfd);
#endif
    if (sockfd < 0)
    {
    	sys_sockerror("socket");
    	return (0);
    }
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;

#ifdef UNIX
    /* if we don`t use REUSEADDR we have to wait under unix until the address gets freed
       after a close ... this can be very annoaing when working with netsend/netreceive GG
    */
    sockopt = 1;    
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof(int)) < 0)
    	post("setsockopt failed\n");
#endif

    	/* assign server port number */
    server.sin_port = htons((u_short)portno);
    post("listening to port number %d", portno);

    	/* name the socket */
    if (bind(sockfd, (struct sockaddr *)&server, sizeof(server)) < 0)
    {
    	sys_sockerror("bind");
    	sys_closesocket(sockfd);
    	return (0);
    }
    if (udp)	    /* datagram protocol */
    {
	x = (t_netreceive *)pd_new(netreceive_class);

	sys_addpollfn(sockfd, (t_fdpollfn)socketreceiver_read, 0);
    }
    else    	/* streaming protocol */
    {
	if (listen(sockfd, 5) < 0)
	{
    	    sys_sockerror("listen");
    	    sys_closesocket(sockfd);
    	    return (0);
	}

	x = (t_netreceive *)pd_new(netreceive_class);

	sys_addpollfn(sockfd, (t_fdpollfn)netreceive_connectpoll, x);
    	outlet_new(&x->x_obj, &s_float);
    }
    x->x_connectsocket = sockfd;
    x->x_nconnections = 0;
    x->x_udp = udp;

    x->x_fdlist = NULL;

    return (x);
}

static void netreceive_free(t_netreceive *x)
{
    	/* LATER make me clean up open connections */

    while (x->x_fdlist) {
	 sys_rmpollfn(x->x_fdlist->fd);
	 sys_closesocket(x->x_fdlist->fd);
	 x->x_fdlist = fdlist_remove(x->x_fdlist);
    }

    sys_rmpollfn(x->x_connectsocket);
    sys_closesocket(x->x_connectsocket);

}

static void netreceive_setup(void)
{
    netreceive_class = class_new(gensym("netreceive"),
    	(t_newmethod)netreceive_new, (t_method)netreceive_free,
    	sizeof(t_netreceive), CLASS_NOINLET, A_DEFFLOAT, A_DEFFLOAT, 0);
}


void x_net_setup(void)
{
    netsend_setup();
    netreceive_setup();
}

