/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Pd side of the Pd/Pd-gui interface. */

#include "m_imp.h"
#ifdef UNIX
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/time.h>
#endif
#ifdef HAVE_BSTRING_H
#include <bstring.h>
#endif
#ifdef NT
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <winsock.h>
typedef int pid_t;
#define EADDRINUSE WSAEADDRINUSE
#endif
#include <stdarg.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifdef MACOSX
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <stdlib.h>
#endif

extern char pd_version[];

typedef struct _fdpoll
{
    int fdp_fd;
    t_fdpollfn fdp_fn;
    void *fdp_ptr;
} t_fdpoll;

#define INBUFSIZE 4096

struct _socketreceiver
{
    char *sr_inbuf;
    int sr_inhead;
    int sr_intail;
    void *sr_owner;
    int sr_udp;
    t_socketnotifier sr_notifier;
    t_socketreceivefn sr_socketreceivefn;
};

static int sys_nfdpoll;
static t_fdpoll *sys_fdpoll;
static int sys_maxfd;
static int sys_guisock;

static t_binbuf *inbinbuf;
static t_socketreceiver *sys_socketreceiver;
extern int sys_addhist(int phase);

/* thread patch by ydegoyon@free.fr */
#define UPDATER_POLLING_DELAY 10000000 /* polling delay in nano seconds ( 10ms ) */

typedef struct _guichunk {
    char *s_chunk;
    struct _guichunk *p_next;
} t_guichunk;
static t_guichunk *sys_chunkslist=NULL;
static t_guichunk *sys_nextchunk=NULL;

static pthread_t sys_updater=-1;       /* thread id for the update child  */   
static pthread_attr_t sys_updater_attr;    /* thread attributes for update child */

static t_int sys_message_counter=0; /* counter of all pending messages */
static t_int sys_message_size=0;    /* size of all pending messages */

static pthread_mutex_t sys_update_mutex;
/* end thread patch by ydegoyon@free.fr */

void sys_sockerror(char *s)
{
#ifdef NT
    int err = WSAGetLastError();
    if (err == 10054) return;
    else if (err == 10044)
    {
    	fprintf(stderr,
	    "Warning: you might not have TCP/IP \"networking\" turned on\n");
	fprintf(stderr, "which is needed for Pd to talk to its GUI layer.\n");
    }
#endif
#ifdef UNIX
    int err = errno;
#endif
    fprintf(stderr, "%s: %s (%d)\n", s, strerror(err), err);
}

void sys_addpollfn(int fd, t_fdpollfn fn, void *ptr)
{
    int nfd = sys_nfdpoll;
    int size = nfd * sizeof(t_fdpoll);
    t_fdpoll *fp;
    sys_fdpoll = (t_fdpoll *)t_resizebytes(sys_fdpoll, size,
    	size + sizeof(t_fdpoll));
    fp = sys_fdpoll + nfd;
    fp->fdp_fd = fd;
    fp->fdp_fn = fn;
    fp->fdp_ptr = ptr;
    sys_nfdpoll = nfd + 1;
    if (fd >= sys_maxfd) sys_maxfd = fd + 1;
}

void sys_rmpollfn(int fd)
{
    int nfd = sys_nfdpoll;
    int i, size = nfd * sizeof(t_fdpoll);
    t_fdpoll *fp;
    for (i = nfd, fp = sys_fdpoll; i--; fp++)
    {
    	if (fp->fdp_fd == fd)
    	{
    	    while (i--)
    	    {
    	    	fp[0] = fp[1];
    	    	fp++;
    	    }
	    sys_fdpoll = (t_fdpoll *)t_resizebytes(sys_fdpoll, size,
    		size - sizeof(t_fdpoll));
	    sys_nfdpoll = nfd - 1;
	    return;
    	}
    }
    post("warning: %d removed from poll list but not found", fd);
}

static int sys_domicrosleep(int microsec, int pollem)
{
    struct timeval timout;
    int i, didsomething = 0;
    t_fdpoll *fp;
    timout.tv_sec = 0;
    timout.tv_usec = microsec;
    if (pollem)
    {
    	fd_set readset, writeset, exceptset;
	FD_ZERO(&writeset);
	FD_ZERO(&readset);
	FD_ZERO(&exceptset);
    	for (fp = sys_fdpoll, i = sys_nfdpoll; i--; fp++)
    	    FD_SET(fp->fdp_fd, &readset);
    	select(sys_maxfd+1, &readset, &writeset, &exceptset, &timout);
	for (i = 0; i < sys_nfdpoll; i++)
    	    if (FD_ISSET(sys_fdpoll[i].fdp_fd, &readset))
	{
    	    (*sys_fdpoll[i].fdp_fn)(sys_fdpoll[i].fdp_ptr, sys_fdpoll[i].fdp_fd);
    	    didsomething = 1;
	}
	return (didsomething);
    }
    else
    {
    	select(0, 0, 0, 0, &timout);
	return (0);
    }
}

void sys_microsleep(int microsec)
{
    sys_domicrosleep(microsec, 1);
}

t_socketreceiver *socketreceiver_new(void *owner, t_socketnotifier notifier,
    t_socketreceivefn socketreceivefn, int udp)
{
    t_socketreceiver *x = (t_socketreceiver *)getbytes(sizeof(*x));
    x->sr_inhead = x->sr_intail = 0;
    x->sr_owner = owner;
    x->sr_notifier = notifier;
    x->sr_socketreceivefn = socketreceivefn;
    x->sr_udp = udp;
    if (!(x->sr_inbuf = malloc(INBUFSIZE))) bug("t_socketreceiver");;
    return (x);
}

void socketreceiver_free(t_socketreceiver *x)
{
    free(x->sr_inbuf);
    freebytes(x, sizeof(*x));
}

    /* this is in a separately called subroutine so that the buffer isn't
    sitting on the stack while the messages are getting passed. */
static int socketreceiver_doread(t_socketreceiver *x)
{
    char messbuf[INBUFSIZE], *bp = messbuf;
    int indx;
    int inhead = x->sr_inhead;
    int intail = x->sr_intail;
    char *inbuf = x->sr_inbuf;
    if (intail == inhead) return (0);
    for (indx = intail; indx != inhead; indx = (indx+1)&(INBUFSIZE-1))
    {
    	    /* if we hit a semi that isn't preceeded by a \, it's a message
    	    boundary.  LATER we should deal with the possibility that the
    	    preceeding \ might itself be escaped! */
    	char c = *bp++ = inbuf[indx];
    	if (c == ';' && (!indx || inbuf[indx-1] != '\\'))
    	{
    	    intail = (indx+1)&(INBUFSIZE-1);
    	    binbuf_text(inbinbuf, messbuf, bp - messbuf);
    	    if (sys_debuglevel & DEBUG_MESSDOWN)
    	    {
    	    	write(2,  messbuf, bp - messbuf);
    	    	write(2, "\n", 1);
    	    }
    	    x->sr_inhead = inhead;
    	    x->sr_intail = intail;
    	    return (1);
    	}
    }
    return (0);
}

static void socketreceiver_getudp(t_socketreceiver *x, int fd)
{
    char buf[INBUFSIZE+1];
    int ret = recv(fd, buf, INBUFSIZE, 0);
    if (ret < 0)
    {
	sys_sockerror("recv");
	sys_rmpollfn(fd);
	sys_closesocket(fd);
    }
    else if (ret > 0)
    {
	buf[ret] = 0;
#if 0
	post("%s", buf);
#endif
    	if (buf[ret-1] != '\n')
	{
#if 0
	    buf[ret] = 0;
	    error("dropped bad buffer %s\n", buf);
#endif
	}
	else
	{
	    char *semi = strchr(buf, ';');
	    if (semi) 
	    	*semi = 0;
    	    binbuf_text(inbinbuf, buf, strlen(buf));
	    outlet_setstacklim();
	    if (x->sr_socketreceivefn)
		(*x->sr_socketreceivefn)(x->sr_owner, inbinbuf);
    	    else bug("socketreceiver_getudp");
    	}
    }
}

void socketreceiver_read(t_socketreceiver *x, int fd)
{
    if (x->sr_udp)   /* UDP ("datagram") socket protocol */
    	socketreceiver_getudp(x, fd);
    else  /* TCP ("streaming") socket protocol */
    {
	char *semi;
	int readto =
	    (x->sr_inhead >= x->sr_intail ? INBUFSIZE : x->sr_intail-1);
	int ret;

    	    /* the input buffer might be full.  If so, drop the whole thing */
	if (readto == x->sr_inhead)
	{
    	    fprintf(stderr, "pd: dropped message from gui\n");
    	    x->sr_inhead = x->sr_intail = 0;
    	    readto = INBUFSIZE;
	}
	else
	{
	    ret = recv(fd, x->sr_inbuf + x->sr_inhead,
	    	readto - x->sr_inhead, 0);
	    if (ret < 0)
	    {
		sys_sockerror("recv");
		if (x == sys_socketreceiver) sys_bail(1);
		else
		{
	    	    if (x->sr_notifier) (*x->sr_notifier)(x->sr_owner);
	    	    sys_rmpollfn(fd);
	    	    sys_closesocket(fd);
		}
	    }
	    else if (ret == 0)
	    {
		if (x == sys_socketreceiver)
		{
    	    	    fprintf(stderr, "pd: exiting\n");
    	    	    sys_bail(0);
		}
		else
		{
	    	    post("EOF on socket %d\n", fd);
	            if (x->sr_notifier) (*x->sr_notifier)(x->sr_owner);
	    	    sys_rmpollfn(fd);
	    	    sys_closesocket(fd);
		}
	    }
	    else
	    {
    		x->sr_inhead += ret;
    		if (x->sr_inhead >= INBUFSIZE) x->sr_inhead = 0;
    		while (socketreceiver_doread(x))
		{
		    outlet_setstacklim();
		    if (x->sr_socketreceivefn)
		    	(*x->sr_socketreceivefn)(x->sr_owner, inbinbuf);
    		    else binbuf_eval(inbinbuf, 0, 0, 0);
 	    	}
	    }
	}
    }
}

void sys_closesocket(int fd)
{
#ifdef UNIX
    close(fd);
#endif
#ifdef NT
    closesocket(fd);
#endif
}

/* thread patch by ydegoyon@free.fr */
static void *sys_dosend(void *tdata)
{ 
  int length, written, res, histwas;
  struct timespec ts;
  t_guichunk *pnext;

    if (sys_nogui)
    	return NULL;

    ts.tv_sec=0;
    ts.tv_nsec=UPDATER_POLLING_DELAY;

    for (;;)
    {
      while ( sys_chunkslist != NULL )
      {
        // fprintf(stderr, "updater sending : %s", sys_chunkslist->s_chunk);
        histwas = sys_addhist(4);
        if (sys_debuglevel & DEBUG_MESSUP)
    	  fprintf(stderr, "%s", sys_chunkslist->s_chunk);
        written = 0;
        length = strlen(sys_chunkslist->s_chunk);
        while (1)
        {
    	  res = send(sys_guisock, sys_chunkslist->s_chunk + written, length, 0);
    	  if (res < 0)
    	  {
    	    // closing the socket will end up this thread
    	    fprintf(stderr, "updater thread : �� � : ending...\n");
    	    exit(0);
    	  }
    	  else
    	  {
    	    written += res;
    	    if (written >= length)
	    	break;
    	  }
        }

        // need exclusive access to modify the list
        if ( pthread_mutex_lock( &sys_update_mutex ) < 0 ) 
        {
    	    fprintf(stderr, "pd updater mutex locking error");
            perror( "pthread_mutex_lock" );
           
        }

        // free sent data
        sys_message_size -= strlen(sys_chunkslist->s_chunk) + 1;
        freebytes(  sys_chunkslist->s_chunk, strlen(sys_chunkslist->s_chunk) + 1 );
        pnext = sys_chunkslist->p_next;
        sys_message_size -=  sizeof( t_guichunk );
        if ( sys_chunkslist == sys_nextchunk ) sys_nextchunk = NULL;
        freebytes(  sys_chunkslist, sizeof( t_guichunk ) );
        sys_chunkslist = pnext;
        sys_message_counter--;
        if ( ( sys_message_counter != 0 ) && ( sys_message_counter % 1000 == 0 ) )
        {
           fprintf(stderr, "messages pending : %d : size : %d\n", sys_message_counter, sys_message_size);
        }

        // ok, list is modified
        if ( pthread_mutex_unlock( &sys_update_mutex ) < 0 ) 
        {
    	    fprintf(stderr, "pd updater mutex unlocking error");
            perror( "pthread_mutex_unlock" );
        }

        sys_addhist(histwas);
      }


      if ( nanosleep( &ts, NULL ) < 0 )
      {
    	fprintf(stderr, "Pd: updater sleping error\n");
        perror( "nanosleep" );
      }

    }
} 

void sys_gui(char *s)
{
   char *newchunk;

   // fprintf(stderr, "storing : %s size=%d\n", s, sys_message_size);
   newchunk = (char*) malloc(strlen (s) + 1 );
   sys_message_size += strlen(s) + 1;
   sys_message_counter++;

   if ( ( sys_message_counter != 0 ) && ( sys_message_counter % 1000 == 0 ) )
   {
      fprintf(stderr, "messages pending : %d : size : %d\n", sys_message_counter, sys_message_size);
   }
   
   if ( !newchunk )
   {
     fprintf(stderr, "Pd: could not allocate buffer...message to the GUI LOST!!!!!!\n"); 
   }

   sprintf( newchunk, "%s", s );

   // need exclusive access to modify the list
   if ( pthread_mutex_lock( &sys_update_mutex ) < 0 ) 
   {
      fprintf(stderr, "pd updater mutex locking error");
      perror( "pthread_mutex_lock" );
   }
           
   if ( sys_chunkslist == NULL )
   {
      sys_chunkslist = (t_guichunk*) malloc( sizeof( t_guichunk ) );
      sys_message_size +=  sizeof( t_guichunk );
      sys_chunkslist->s_chunk = newchunk;
      sys_chunkslist->p_next = NULL;
      sys_nextchunk = sys_chunkslist;
   }
   else
   {
      // find the end of the list
      sys_nextchunk->p_next = (t_guichunk*) malloc( sizeof( t_guichunk ) ); 
      sys_message_size +=  sizeof( t_guichunk );
      sys_nextchunk->p_next->s_chunk = newchunk;
      sys_nextchunk->p_next->p_next = NULL;
      sys_nextchunk=sys_nextchunk->p_next;
   }

   // ok, list is modified
   if ( pthread_mutex_unlock( &sys_update_mutex ) < 0 ) 
   {
       fprintf(stderr, "pd updater mutex unlocking error");
       perror( "pthread_mutex_unlock" );
   }
   
}

/* end thread patch by ydegoyon@free.fr */

/* LATER should do a bounds check -- but how do you get printf to do that?
    See also rtext_senditup() in this regard */

void sys_vgui(char *fmt, ...)
{
    int result, i;
    char buf[2048];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    sys_gui(buf);
    va_end(ap);
}


#define FIRSTPORTNUM 5400

/* -------------- signal handling for UNIX -------------- */

#ifdef UNIX
typedef void (*sighandler_t)(int);

static void sys_signal(int signo, sighandler_t sigfun)
{
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = sigfun;
    memset(&action.sa_mask, 0, sizeof(action.sa_mask));
#ifdef __linux__
    action.sa_restorer = 0;
#endif
    if (sigaction(signo, &action, 0) < 0)
    	perror("sigaction");
}

static void sys_exithandler(int n)
{
    static int trouble = 0;
    if (!trouble)
    {
    	trouble = 1;
    	fprintf(stderr, "Pd: signal %d\n", n);
    	sys_bail(1);

    }
    else _exit(1);
}

static void sys_alarmhandler(int n)
{
    fprintf(stderr, "Pd: system call timed out\n");
}

static void sys_huphandler(int n)
{
    struct timeval timout;
    timout.tv_sec = 0;
    timout.tv_usec = 30000;
    select(1, 0, 0, 0, &timout);
}

void sys_setalarm(int microsec)
{
    struct itimerval gonzo;
#if 0
    fprintf(stderr, "timer %d\n", microsec);
#endif
    gonzo.it_interval.tv_sec = 0;
    gonzo.it_interval.tv_usec = 0;
    gonzo.it_value.tv_sec = 0;
    gonzo.it_value.tv_usec = microsec;
    if (microsec)
    	sys_signal(SIGALRM, sys_alarmhandler);
    else sys_signal(SIGALRM, SIG_IGN);
    setitimer(ITIMER_REAL, &gonzo, 0);
}

#endif

static int sys_watchfd;

void glob_ping(t_pd *dummy)
{
    if (write(sys_watchfd, "\n", 1) < 1)
    {
    	fprintf(stderr, "pd: watchdog process died\n");
	sys_bail(1);
    }
}

static int defaultfontshit[] = {
    8, 5, 9, 10, 6, 10, 12, 7, 13, 14, 9, 17, 16, 10, 19, 24, 15, 28,
    	24, 15, 28};

int sys_startgui(const char *guidir)
{
    pid_t childpid;
    char cmdbuf[4*MAXPDSTRING];
    struct sockaddr_in server;
    int msgsock;
    char buf[15];
    int len = sizeof(server);
    int ntry = 0, portno = FIRSTPORTNUM;
    int xsock = -1;
#ifdef NT
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
#endif
#ifdef UNIX
    int stdinpipe[2];
#endif
    /* create an empty FD poll list */
    sys_fdpoll = (t_fdpoll *)t_getbytes(0);
    sys_nfdpoll = 0;
    inbinbuf = binbuf_new();

#ifdef UNIX
    signal(SIGHUP, sys_huphandler);
    signal(SIGINT, sys_exithandler);
    signal(SIGQUIT, sys_exithandler);
    signal(SIGILL, sys_exithandler);
    signal(SIGIOT, sys_exithandler);
    signal(SIGFPE, SIG_IGN);
    /* signal(SIGILL, sys_exithandler);
    signal(SIGBUS, sys_exithandler);
    signal(SIGSEGV, sys_exithandler); */
    signal(SIGPIPE, sys_exithandler);
    signal(SIGALRM, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
#ifdef __linux__
    signal(SIGSTKFLT, sys_exithandler);
#endif
#endif
#ifdef NT
    if (WSAStartup(version, &nobby)) sys_sockerror("WSAstartup");
#endif

    if (sys_nogui)
    {
    	    /* fake the GUI's message giving cwd and font sizes; then
	    skip starting the GUI up. */
    	t_atom zz[19];
	int i;
#ifdef NT
    	if (GetCurrentDirectory(MAXPDSTRING, cmdbuf) == 0)
	    strcpy(cmdbuf, ".");
#endif
#ifdef UNIX
    	if (!getcwd(cmdbuf, MAXPDSTRING))
	    strcpy(cmdbuf, ".");
    	
#endif
    	SETSYMBOL(zz, gensym(cmdbuf));
	for (i = 1; i < 22; i++)
	    SETFLOAT(zz + i, defaultfontshit[i-1]);
    	glob_initfromgui(0, 0, 22, zz);
    }
    else
    {
#ifdef NT
    	char scriptbuf[MAXPDSTRING+30], wishbuf[MAXPDSTRING+30], portbuf[80];
    	int spawnret;

#endif
    	int intarg;

	/* create a socket */
	xsock = socket(AF_INET, SOCK_STREAM, 0);
	if (xsock < 0) sys_sockerror("socket");
#if 0
	intarg = 0;
	if (setsockopt(xsock, SOL_SOCKET, SO_SNDBUF,
    	    &intarg, sizeof(intarg)) < 0)
    		post("setsockopt (SO_RCVBUF) failed\n");
	intarg = 0;
	if (setsockopt(xsock, SOL_SOCKET, SO_RCVBUF,
    	    &intarg, sizeof(intarg)) < 0)
    		post("setsockopt (SO_RCVBUF) failed\n");
#endif
	intarg = 1;
	if (setsockopt(xsock, IPPROTO_TCP, TCP_NODELAY,
    	    &intarg, sizeof(intarg)) < 0)
    		post("setsockopt (TCP_NODELAY) failed\n");
	
	
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;

	/* assign server port number */
	server.sin_port =  htons((unsigned short)portno);

	/* name the socket */
	while (bind(xsock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
#ifdef NT
    	    int err = WSAGetLastError();
#endif
#ifdef UNIX
    	    int err = errno;
#endif
    	    if ((ntry++ > 20) || (err != EADDRINUSE))
    	    {
		perror("bind");
		fprintf(stderr,
		    "Pd needs your machine to be configured with\n");
		fprintf(stderr,
	    	  "'networking' turned on (see Pd's html doc for details.)\n");
		exit(1);
    	    }
    	    server.sin_port = htons((unsigned short)(++portno));
	}

	if (sys_verbose) fprintf(stderr, "port %d\n", portno);

	sys_socketreceiver = socketreceiver_new(0, 0, 0, 0);

#ifdef UNIX
	childpid = fork();
	if (childpid < 0)
	{
            if (errno) perror("sys_startgui");
    	    else fprintf(stderr, "sys_startgui failed\n");
    	    return (1);
	}
	else if (!childpid)	    	    	/* we're the child */
	{
    	    seteuid(getuid());  	/* lose setuid priveliges */
#ifndef MACOSX
    	    	/* the wish process in Unix will make a wish shell and
		    read/write standard in and out unless we close the
		    file descriptors.  Somehow this doesn't make the MAC OSX
			version of Wish happy...*/
    	    if (pipe(stdinpipe) < 0)
	    	sys_sockerror("pipe");
	    else
	    {
	    	if (stdinpipe[0] != 0)
		{
		    close (0);
		    dup2(stdinpipe[0], 0);
		    close(stdinpipe[0]);
		}
	    }
#endif
    	    if (!sys_guicmd)
	    {
#ifdef MACOSX
                char *homedir = getenv("HOME"), filename[250];
                struct stat statbuf;
                if (!homedir || strlen(homedir) > 150)
                    goto nexttry;
                sprintf(filename,
                    "%s/Applications/Wish shell.app/Contents/MacOS/Wish Shell",
                        homedir);
                
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
            nexttry:
                strcpy(filename, 
                    "/Applications/Wish Shell.app/Contents/MacOS/Wish Shell");
            foundit:
    	    	sprintf(cmdbuf, "\"%s\" %s/pd.tk %d\n", filename, guidir, portno);
#else
    		sprintf(cmdbuf,
"TCL_LIBRARY=\"%s/tcl/library\" TK_LIBRARY=\"%s/tk/library\" \
 \"%s/pd-gui\" %d\n",
    	    	    sys_libdir->s_name, sys_libdir->s_name, guidir, portno);
#endif
    		sys_guicmd = cmdbuf;
	    }
	    if (sys_verbose) fprintf(stderr, "%s", sys_guicmd);
    	    execl("/bin/sh", "sh", "-c", sys_guicmd, 0);
    	    perror("pd: exec");
    	    _exit(1);
    	}
#endif /* UNIX */

#ifdef NT
    	    /* in NT land "guipath" is unused; we just do everything from
	    the libdir. */
    	fprintf(stderr, "%s\n", sys_libdir->s_name);
    	
    	strcpy(scriptbuf, "\"");
    	strcat(scriptbuf, sys_libdir->s_name);
    	strcat(scriptbuf, "/bin/pd.tk\"");
    	sys_bashfilename(scriptbuf, scriptbuf);
    	
		sprintf(portbuf, "%d", portno);

    	strcpy(wishbuf, sys_libdir->s_name);
    	strcat(wishbuf, "/bin/wish83.exe");
    	sys_bashfilename(wishbuf, wishbuf);
    	
     	spawnret = _spawnl(P_NOWAIT, wishbuf, "wish83", scriptbuf, portbuf, 0);
    	if (spawnret < 0)
    	{
    	    perror("spawnl");
    	    fprintf(stderr, "%s: couldn't load TCL\n", wishbuf);
    	    exit(1);
    	}

#endif /* NT */
    }

#ifdef UNIX
    	/* now that we've spun off the child process we can promote
	our process's priority, if we happen to be root.  */
    if (sys_hipriority)
    {
	if (!getuid() || !geteuid())
	{
	    	/* To prevent lockup, we fork off a watchdog process with
		higher real-time priority than ours.  The GUI has to send
		a stream of ping messages to the watchdog THROUGH the Pd
		process which has to pick them up from the GUI and forward
		them.  If any of these things aren't happening the watchdog
		starts sending "stop" and "cont" signals to the Pd process
		to make it timeshare with the rest of the system.  (Version
		0.33P2 : if there's no GUI, the watchdog pinging is done
		from the scheduler idle routine in this process instead.) */
 
	    int pipe9[2], watchpid;
    	    if (pipe(pipe9) < 0)
	    {
		seteuid(getuid());  	/* lose setuid priveliges */
		sys_sockerror("pipe");
		return (1);
	    }
	    watchpid = fork();
	    if (watchpid < 0)
	    {
	    	seteuid(getuid());  	/* lose setuid priveliges */
        	if (errno)
		    perror("sys_startgui");
    		else fprintf(stderr, "sys_startgui failed\n");
    		return (1);
	    }
	    else if (!watchpid)	    	    /* we're the child */
	    {
    	    	sys_set_priority(1);
	    	seteuid(getuid());  	/* lose setuid priveliges */
    	    	if (pipe9[1] != 0)
		{
		    dup2(pipe9[0], 0);
    		    close(pipe9[0]);
    		}
		close(pipe9[1]);

    		sprintf(cmdbuf, "%s/pd-watchdog\n", guidir);
    		if (sys_verbose) fprintf(stderr, "%s", cmdbuf);
    		execl("/bin/sh", "sh", "-c", cmdbuf, 0);
    		perror("pd: exec");
    		_exit(1);
	    }
	    else    	        	    /* we're the parent */
	    {
    	    	sys_set_priority(0);
	    	seteuid(getuid());  	/* lose setuid priveliges */
    	    	close(pipe9[0]);
		sys_watchfd = pipe9[1];
		    /* We also have to start the ping loop in the GUI;
		    this is done later when the socket is open. */
	    }
    	}
	else
	{
	    post("realtime setting failed because not root\n");
	    sys_hipriority = 0;
	}
    }

    seteuid(getuid());  	/* lose setuid priveliges */
#endif /* UNIX */

#ifdef NT
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
        fprintf(stderr, "pd: couldn't set high priority class\n");
#endif

    if (!sys_nogui)
    {
	if (sys_verbose)
    	    fprintf(stderr, "Waiting for connection request... \n");
	if (listen(xsock, 5) < 0) sys_sockerror("listen");

	sys_guisock = accept(xsock, (struct sockaddr *) &server, &len);
#ifdef OOPS
	close(xsock);
#endif
	if (sys_guisock < 0) sys_sockerror("accept");
	sys_addpollfn(sys_guisock, (t_fdpollfn)socketreceiver_read,
    	    sys_socketreceiver);

	if (sys_verbose)
    	    fprintf(stderr, "... connected\n");

/* thread patch by ydegoyon@free.fr */
    // launch update thread
    if ( pthread_attr_init( &sys_updater_attr ) < 0 ) {
       post( "sonogram~ : could not launch update thread" );
       perror( "pthread_attr_init" );
       exit(-1);
    }
    if ( pthread_attr_setdetachstate( &sys_updater_attr, PTHREAD_CREATE_DETACHED ) < 0 ) {
       post( "sonogram~ : could not launch update thread" );
       perror( "pthread_attr_setdetachstate" );
       exit(-1);
    }
    if ( pthread_create( &sys_updater, &sys_updater_attr, sys_dosend, NULL ) < 0 ) {
       post( "sonogram~ : could not launch update thread" );
       perror( "pthread_create" );
       exit (-1);
    }
    else
    {
       post( "sonogram~ : updater thread %d launched", (int)sys_updater );
    }
    // no error return code
    pthread_mutex_init( &sys_update_mutex, NULL ); 
/* end thread patch by ydegoyon@free.fr */

	    /* here is where we start the pinging. */
	if (sys_hipriority)
    	    sys_gui("pdtk_watchdog\n");

	sys_vgui("pdtk_pd_startup {%s}\n", pd_version); 
    }
    return (0);

}


static int sys_poll_togui(void)
{
    /* LATER use this to flush output buffer to gui */
    return (0);
}

int sys_pollgui(void)
{
    return (sys_domicrosleep(0, 1) || sys_poll_togui());
}

/* LATER try to save dirty documents */
void sys_bail(int n)
{
    static int reentered = 0;
    if (!reentered)
    {
    	reentered = 1;
    	sys_close_audio();
	sys_close_midi();
    }
    _exit(n);
}

void glob_quit(void *dummy)
{
    sys_vgui("exit\n");
    if (!sys_nogui)
    	close(sys_guisock);
    sys_bail(0); 
}

