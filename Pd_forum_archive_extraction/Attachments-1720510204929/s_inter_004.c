/* Copyright (c) 1997-1999 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* Pd side of the Pd/Pd-gui interface.  Also, some system interface routines
that didn't really belong anywhere. */

#include "m_pd.h"
#include "s_stuff.h"
#include "m_imp.h"
#include "g_canvas.h"   /* for GUI queueing stuff */
#ifndef MSW
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <netdb.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/resource.h>
#endif
#ifdef HAVE_BSTRING_H
#include <bstring.h>
#endif
#ifdef MSW
#include <io.h>
#include <fcntl.h>
#include <process.h>
#include <winsock.h>
#include <windows.h>
typedef int pid_t;
typedef int socklen_t;
#define EADDRINUSE WSAEADDRINUSE
#endif
#include <stdarg.h>
#include <signal.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>
#else
#include <stdlib.h>
#endif

#define DEBUG_MESSUP 1      /* messages up from pd to pd-gui */
#define DEBUG_MESSDOWN 2    /* messages down from pd-gui to pd */

#ifndef PDBINDIR
#define PDBINDIR "bin/"
#endif

#ifndef WISHAPP
#define WISHAPP "wish84.exe"
#endif

#ifdef __linux__
#define LOCALHOST "127.0.0.1"
#else
#define LOCALHOST "localhost"
#endif

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

extern char *pd_version;
extern int sys_guisetportnumber;
extern char sys_font[]; /* tb: typeface */

static int sys_nfdpoll;
static t_fdpoll *sys_fdpoll;
static int sys_maxfd;
static int sys_guisock;

static t_binbuf *inbinbuf;
static t_socketreceiver *sys_socketreceiver;
extern int sys_addhist(int phase);

/* ----------- functions for timing, signals, priorities, etc  --------- */

#ifdef MSW
static LARGE_INTEGER nt_inittime;
static double nt_freq = 0;

static void sys_initntclock(void)
{
    LARGE_INTEGER f1;
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (!QueryPerformanceFrequency(&f1))
    {
          fprintf(stderr, "pd: QueryPerformanceFrequency failed\n");
          f1.QuadPart = 1;
    }
    nt_freq = f1.QuadPart;
    nt_inittime = now;
}

#if 0
    /* this is a version you can call if you did the QueryPerformanceCounter
    call yourself.  Necessary for time tagging incoming MIDI at interrupt
    level, for instance; but we're not doing that just now. */

double nt_tixtotime(LARGE_INTEGER *dumbass)
{
    if (nt_freq == 0) sys_initntclock();
    return (((double)(dumbass->QuadPart - nt_inittime.QuadPart)) / nt_freq);
}
#endif
#endif /* MSW */

    /* get "real time" in seconds; take the
    first time we get called as a reference time of zero. */
double sys_getrealtime(void)    
{
#ifndef MSW
    static struct timeval then;
    struct timeval now;
    gettimeofday(&now, 0);
    if (then.tv_sec == 0 && then.tv_usec == 0) then = now;
    return ((now.tv_sec - then.tv_sec) +
        (1./1000000.) * (now.tv_usec - then.tv_usec));
#else
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    if (nt_freq == 0) sys_initntclock();
    return (((double)(now.QuadPart - nt_inittime.QuadPart)) / nt_freq);
#endif
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
#ifdef MSW
        if (sys_maxfd == 0)
                Sleep(microsec/1000);
        else
#endif
        select(sys_maxfd+1, &readset, &writeset, &exceptset, &timout);
        for (i = 0; i < sys_nfdpoll; i++)
            if (FD_ISSET(sys_fdpoll[i].fdp_fd, &readset))
        {
#ifdef THREAD_LOCKING
            sys_lock();
#endif
            (*sys_fdpoll[i].fdp_fn)(sys_fdpoll[i].fdp_ptr, sys_fdpoll[i].fdp_fd);
#ifdef THREAD_LOCKING
            sys_unlock();
#endif
            didsomething = 1;
        }
        return (didsomething);
    }
    else
    {
#ifdef MSW
        if (sys_maxfd == 0)
              Sleep(microsec/1000);
        else
#endif
        select(0, 0, 0, 0, &timout);
        return (0);
    }
}

void sys_microsleep(int microsec)
{
    sys_domicrosleep(microsec, 1);
}

#ifdef UNISTD
typedef void (*sighandler_t)(int);

static void sys_signal(int signo, sighandler_t sigfun)
{
    struct sigaction action;
    action.sa_flags = 0;
    action.sa_handler = sigfun;
    memset(&action.sa_mask, 0, sizeof(action.sa_mask));
#if 0  /* GG says: don't use that */
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

#ifdef __linux

#if defined(_POSIX_PRIORITY_SCHEDULING) || defined(_POSIX_MEMLOCK)
#include <sched.h>
#endif

void sys_set_priority(int higher) 
{
#ifdef _POSIX_PRIORITY_SCHEDULING
    struct sched_param par;
    int p1 ,p2, p3;
    p1 = sched_get_priority_min(SCHED_FIFO);
    p2 = sched_get_priority_max(SCHED_FIFO);
#ifdef USEAPI_JACK    
    p3 = (higher ? p1 + 7 : p1 + 5);
#else
    p3 = (higher ? p2 - 1 : p2 - 3);
#endif
    par.sched_priority = p3;
    if (sched_setscheduler(0,SCHED_FIFO,&par) != -1)
       fprintf(stderr, "priority %d scheduling enabled.\n", p3);
#endif

#ifdef REALLY_POSIX_MEMLOCK /* this doesn't work on Fedora 4, for example. */
#ifdef _POSIX_MEMLOCK
    /* tb: force memlock to physical memory { */
    {
        struct rlimit mlock_limit;
        mlock_limit.rlim_cur=0;
        mlock_limit.rlim_max=0;
        setrlimit(RLIMIT_MEMLOCK,&mlock_limit);
    }
    /* } tb */
    if (mlockall(MCL_FUTURE) != -1) 
        fprintf(stderr, "memory locking enabled.\n");
#endif
#endif
}

#endif /* __linux__ */

#ifdef IRIX             /* hack by <olaf.matthes@gmx.de> at 2003/09/21 */

#if defined(_POSIX_PRIORITY_SCHEDULING) || defined(_POSIX_MEMLOCK)
#include <sched.h>
#endif

void sys_set_priority(int higher)
{
#ifdef _POSIX_PRIORITY_SCHEDULING
    struct sched_param par;
        /* Bearing the table found in 'man realtime' in mind, I found it a */
        /* good idea to use 192 as the priority setting for Pd. Any thoughts? */
    if (higher)
                par.sched_priority = 250;       /* priority for watchdog */
    else
                par.sched_priority = 192;       /* priority for pd (DSP) */

    if (sched_setscheduler(0, SCHED_FIFO, &par) != -1)
        fprintf(stderr, "priority %d scheduling enabled.\n", par.sched_priority);
#endif

#ifdef _POSIX_MEMLOCK
    if (mlockall(MCL_FUTURE) != -1) 
        fprintf(stderr, "memory locking enabled.\n");
#endif
}
/* end of hack */
#endif /* IRIX */

/* ------------------ receiving incoming messages over sockets ------------- */

void sys_sockerror(char *s)
{
#ifdef MSW
    int err = WSAGetLastError();
    if (err == 10054) return;
    else if (err == 10044)
    {
        fprintf(stderr,
            "Warning: you might not have TCP/IP \"networking\" turned on\n");
        fprintf(stderr, "which is needed for Pd to talk to its GUI layer.\n");
    }
#else
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

void sys_exit(void);

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
                    sys_exit();
                    return;
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
#ifdef UNISTD
    close(fd);
#endif
#ifdef MSW
    closesocket(fd);
#endif
}

/* ---------------------- sending messages to the GUI ------------------ */
#define GUI_ALLOCCHUNK 8192
#define GUI_UPDATESLICE 512 /* how much we try to do in one idle period */
#define GUI_BYTESPERPING 1024 /* how much we send up per ping */

typedef struct _guiqueue
{
    void *gq_client;
    t_glist *gq_glist;
    t_guicallbackfn gq_fn;
    struct _guiqueue *gq_next;
} t_guiqueue;

static t_guiqueue *sys_guiqueuehead;

/* thread patch by ydegoyon@free.fr */
#define UPDATER_POLLING_DELAY 10000000 /* polling delay in nano seconds ( 10ms ) */

typedef struct _guichunk {
    char *s_chunk;
    struct _guichunk *p_next;
} t_guichunk;
static t_guichunk *sys_chunkslist=NULL;
static t_guichunk *sys_nextchunk=NULL;

static pthread_t sys_updater; // = -1;       /* thread id for the update child  */
static pthread_attr_t sys_updater_attr;    /* thread attributes for update child */

static int sys_message_counter=0; /* counter of all pending messages */
static int sys_message_size=0;    /* size of all pending messages */

static pthread_mutex_t sys_update_mutex;
/* end thread patch by ydegoyon@free.fr */

static int sys_waitingforping;
static int sys_bytessincelastping;

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
            fprintf(stderr, "updater thread : ·þ ð : ending...\n");
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

#ifdef UNIX
      if ( nanosleep( &ts, NULL ) < 0 )
      {
        fprintf(stderr, "Pd: updater sleping error\n");
        perror( "nanosleep" );
      }
#endif

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

void glob_ping(t_pd *dummy)
{
    sys_waitingforping = 0;
}

static int sys_flushqueue(void )
{
    int wherestop = sys_bytessincelastping + GUI_UPDATESLICE;
    if (wherestop + (GUI_UPDATESLICE >> 1) > GUI_BYTESPERPING)
        wherestop = 0x7fffffff;
    if (sys_waitingforping)
        return (0);
    if (!sys_guiqueuehead)
        return (0);
    while (1)
    {
        if (sys_bytessincelastping >= GUI_BYTESPERPING)
        {
            sys_gui("pdtk_ping\n");
            sys_bytessincelastping = 0;
            sys_waitingforping = 1;
            return (1);
        }
        if (sys_guiqueuehead)
        {
            t_guiqueue *headwas = sys_guiqueuehead;
            sys_guiqueuehead = headwas->gq_next;
            (*headwas->gq_fn)(headwas->gq_client, headwas->gq_glist);
            t_freebytes(headwas, sizeof(*headwas));
            if (sys_bytessincelastping >= wherestop)
                break;
        }
        else break;
    }
    // sys_flushtogui();
    return (1);
}

    /* flush output buffer and update queue to gui in small time slices */
static int sys_poll_togui(void) /* returns 1 if did anything */
{
    if (sys_nogui)
        return (0);
        /* see if there is stuff still in the buffer, if so we
            must have fallen behind, so just try to clear that. */
    // if (sys_flushtogui())
    //     return (1);
        /* if the flush wasn't complete, wait. */
    // if (sys_guibufhead > sys_guibuftail)
    //     return (0);
    
        /* check for queued updates */
    if (sys_flushqueue())
        return (1);
    
    return (0);
}

    /* if some GUI object is having to do heavy computations, it can tell
    us to back off from doing more updates by faking a big one itself. */
void sys_pretendguibytes(int n)
{
    sys_bytessincelastping += n;
}

void sys_queuegui(void *client, t_glist *glist, t_guicallbackfn f)
{
    t_guiqueue **gqnextptr, *gq;
    if (!sys_guiqueuehead)
        gqnextptr = &sys_guiqueuehead;
    else
    {
        for (gq = sys_guiqueuehead; gq->gq_next; gq = gq->gq_next)
            if (gq->gq_client == client)
                return;
        if (gq->gq_client == client)
            return;
        gqnextptr = &gq->gq_next;
    }
    gq = t_getbytes(sizeof(*gq));
    gq->gq_next = 0;
    gq->gq_client = client;
    gq->gq_glist = glist;
    gq->gq_fn = f;
    gq->gq_next = 0;
    *gqnextptr = gq;
}

void sys_unqueuegui(void *client)
{
    t_guiqueue *gq, *gq2;
    if (!sys_guiqueuehead)
        return;
    if (sys_guiqueuehead->gq_client == client)
    {
        t_freebytes(sys_guiqueuehead, sizeof(*sys_guiqueuehead));
        sys_guiqueuehead = 0;
    }
    else for (gq = sys_guiqueuehead; gq2 = gq->gq_next; gq = gq2)
        if (gq2->gq_client == client)
    {
        gq->gq_next = gq2->gq_next;
        t_freebytes(gq2, sizeof(*gq2));
        break;
    }
}

int sys_pollgui(void)
{
    return (sys_domicrosleep(0, 1) || sys_poll_togui());
}



/* --------------------- starting up the GUI connection ------------- */

static int sys_watchfd;

#ifdef __linux__
void glob_watchdog(t_pd *dummy)
{
    if (write(sys_watchfd, "\n", 1) < 1)
    {
        fprintf(stderr, "pd: watchdog process died\n");
        sys_bail(1);
    }
}
#endif

#define FIRSTPORTNUM 5400

static int defaultfontshit[] = {
    8, 5, 9, 10, 6, 10, 12, 7, 13, 14, 9, 17, 16, 10, 19, 24, 15, 28,
        24, 15, 28};
#define NDEFAULTFONT (sizeof(defaultfontshit)/sizeof(*defaultfontshit))

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
#ifdef MSW
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
#endif
#ifdef UNISTD
    int stdinpipe[2];
#endif
    /* create an empty FD poll list */
    sys_fdpoll = (t_fdpoll *)t_getbytes(0);
    sys_nfdpoll = 0;
    inbinbuf = binbuf_new();

#ifdef UNISTD
    signal(SIGHUP, sys_huphandler);
    signal(SIGINT, sys_exithandler);
    signal(SIGQUIT, sys_exithandler);
    signal(SIGILL, sys_exithandler);
    signal(SIGIOT, sys_exithandler);
    signal(SIGFPE, SIG_IGN);
    /* signal(SIGILL, sys_exithandler);
    signal(SIGBUS, sys_exithandler);
    signal(SIGSEGV, sys_exithandler); */
    signal(SIGPIPE, SIG_IGN);
    signal(SIGALRM, SIG_IGN);
#if 0  /* GG says: don't use that */
    signal(SIGSTKFLT, sys_exithandler);
#endif
#endif
#ifdef MSW
    if (WSAStartup(version, &nobby)) sys_sockerror("WSAstartup");
#endif

    if (sys_nogui)
    {
            /* fake the GUI's message giving cwd and font sizes; then
            skip starting the GUI up. */
        t_atom zz[NDEFAULTFONT+2];
        int i;
#ifdef MSW
        if (GetCurrentDirectory(MAXPDSTRING, cmdbuf) == 0)
            strcpy(cmdbuf, ".");
#endif
#ifdef UNISTD
        if (!getcwd(cmdbuf, MAXPDSTRING))
            strcpy(cmdbuf, ".");
        
#endif
        SETSYMBOL(zz, gensym(cmdbuf));
        for (i = 0; i < (int)NDEFAULTFONT; i++)
            SETFLOAT(zz+i+1, defaultfontshit[i]);
        SETFLOAT(zz+NDEFAULTFONT+1,0);
        glob_initfromgui(0, 0, 23, zz);
    }
    else if (sys_guisetportnumber)  /* GUI exists and sent us a port number */
    {
        struct sockaddr_in server;
        struct hostent *hp;
        /* create a socket */
        sys_guisock = socket(AF_INET, SOCK_STREAM, 0);
        if (sys_guisock < 0)
            sys_sockerror("socket");

        /* connect socket using hostname provided in command line */
        server.sin_family = AF_INET;

        hp = gethostbyname(LOCALHOST);

        if (hp == 0)
        {
            fprintf(stderr,
                "localhost not found (inet protocol not installed?)\n");
            exit(1);
        }
        memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

        /* assign client port number */
        server.sin_port = htons((unsigned short)sys_guisetportnumber);

            /* try to connect */
        if (connect(sys_guisock, (struct sockaddr *) &server, sizeof (server))
            < 0)
        {
            sys_sockerror("connecting stream socket");
            exit(1);
        }
    }
    else    /* default behavior: start up the GUI ourselves. */
    {
#ifdef MSW
        char scriptbuf[MAXPDSTRING+30], wishbuf[MAXPDSTRING+30], portbuf[80];
        int spawnret;

#endif
#ifdef MSW
        char intarg;
#else
        int intarg;
#endif

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
#ifndef MSW
                post("setsockopt (TCP_NODELAY) failed\n")
#endif
                    ;
        
        
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;

        /* assign server port number */
        server.sin_port =  htons((unsigned short)portno);

        /* name the socket */
        while (bind(xsock, (struct sockaddr *)&server, sizeof(server)) < 0)
        {
#ifdef MSW
            int err = WSAGetLastError();
#else
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
            portno++;
            server.sin_port = htons((unsigned short)(portno));
        }

        if (sys_verbose) fprintf(stderr, "port %d\n", portno);


#ifdef UNISTD
        childpid = fork();
        if (childpid < 0)
        {
            if (errno) perror("sys_startgui");
            else fprintf(stderr, "sys_startgui failed\n");
            return (1);
        }
        else if (!childpid)                     /* we're the child */
        {
            seteuid(getuid());          /* lose setuid priveliges */
#ifndef __APPLE__
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
#ifdef __APPLE__
                char *homedir = getenv("HOME"), filename[250];
                struct stat statbuf;
                    /* first look for Wish bundled with and renamed "Pd" */
                sprintf(filename, "%s/../../MacOS/Pd", guidir);
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
                if (!homedir || strlen(homedir) > 150)
                    goto nohomedir;
                    /* Look for Wish in user's Applications.  Might or might
                    not be names "Wish Shell", and might or might not be
                    in "Utilities" subdir. */
                sprintf(filename,
                    "%s/Applications/Utilities/Wish shell.app/Contents/MacOS/Wish Shell",
                        homedir);
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
                sprintf(filename,
                    "%s/Applications/Utilities/Wish.app/Contents/MacOS/Wish",
                        homedir);
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
                sprintf(filename,
                    "%s/Applications/Wish shell.app/Contents/MacOS/Wish Shell",
                        homedir);
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
                sprintf(filename,
                    "%s/Applications/Wish.app/Contents/MacOS/Wish",
                        homedir);
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
            nohomedir:
                    /* Perform the same search among system applications. */
                strcpy(filename, 
                    "/Applications/Utilities/Wish Shell.app/Contents/MacOS/Wish Shell");
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
                strcpy(filename, 
                    "/Applications/Utilities/Wish.app/Contents/MacOS/Wish");
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
                strcpy(filename, 
                    "/Applications/Wish Shell.app/Contents/MacOS/Wish Shell");
                if (stat(filename, &statbuf) >= 0)
                    goto foundit;
                strcpy(filename, 
                    "/Applications/Wish.app/Contents/MacOS/Wish");
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
            execl("/bin/sh", "sh", "-c", sys_guicmd, (char*)0);
            perror("pd: exec");
            _exit(1);
        }
#endif /* UNISTD */

#ifdef MSW
            /* in MSW land "guipath" is unused; we just do everything from
            the libdir. */
        /* fprintf(stderr, "%s\n", sys_libdir->s_name); */
        
        strcpy(scriptbuf, "\"");
        strcat(scriptbuf, sys_libdir->s_name);
        strcat(scriptbuf, "/" PDBINDIR "pd.tk\"");
        sys_bashfilename(scriptbuf, scriptbuf);
        
                sprintf(portbuf, "%d", portno);

        strcpy(wishbuf, sys_libdir->s_name);
        strcat(wishbuf, "/" PDBINDIR WISHAPP);
        sys_bashfilename(wishbuf, wishbuf);
        
        spawnret = _spawnl(P_NOWAIT, wishbuf, WISHAPP, scriptbuf, portbuf, 0);
        if (spawnret < 0)
        {
            perror("spawnl");
            fprintf(stderr, "%s: couldn't load TCL\n", wishbuf);
            exit(1);
        }

#endif /* MSW */
    }

#if defined(__linux__) || defined(IRIX)
        /* now that we've spun off the child process we can promote
        our process's priority, if we can and want to.  If not specfied
        (-1), we check root status.  This misses the case where we might
        have permission from a "security module" (linux 2.6) -- I don't
        know how to test for that.  The "-rt" flag must b eset in that
        case. */
    if (sys_hipriority == -1)
        sys_hipriority = (!getuid() || !geteuid());
    
    if (sys_hipriority)
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
            seteuid(getuid());      /* lose setuid priveliges */
            sys_sockerror("pipe");
            return (1);
        }
        watchpid = fork();
        if (watchpid < 0)
        {
            seteuid(getuid());      /* lose setuid priveliges */
            if (errno)
                perror("sys_startgui");
            else fprintf(stderr, "sys_startgui failed\n");
            return (1);
        }
        else if (!watchpid)             /* we're the child */
        {
            sys_set_priority(1);
            seteuid(getuid());      /* lose setuid priveliges */
            if (pipe9[1] != 0)
            {
                dup2(pipe9[0], 0);
                close(pipe9[0]);
            }
            close(pipe9[1]);

            sprintf(cmdbuf, "%s/pd-watchdog\n", guidir);
            if (sys_verbose) fprintf(stderr, "%s", cmdbuf);
            execl("/bin/sh", "sh", "-c", cmdbuf, (char*)0);
            perror("pd: exec");
            _exit(1);
        }
        else                            /* we're the parent */
        {
            sys_set_priority(0);
            seteuid(getuid());      /* lose setuid priveliges */
            close(pipe9[0]);
            sys_watchfd = pipe9[1];
                /* We also have to start the ping loop in the GUI;
                this is done later when the socket is open. */
        }
    }

    seteuid(getuid());          /* lose setuid priveliges */
#endif /* __linux__ */

#ifdef MSW
    if (!SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS))
        fprintf(stderr, "pd: couldn't set high priority class\n");
#endif
#ifdef __APPLE__
    if (sys_hipriority)
    {
        struct sched_param param;
        int policy = SCHED_RR;
        int err;
        param.sched_priority = 80; /* adjust 0 : 100 */

        err = pthread_setschedparam(pthread_self(), policy, &param);
        if (err)
            post("warning: high priority scheduling failed\n");
    }
#endif /* __APPLE__ */

    if (!sys_nogui && !sys_guisetportnumber)
    {
        if (sys_verbose)
            fprintf(stderr, "Waiting for connection request... \n");
        if (listen(xsock, 5) < 0) sys_sockerror("listen");

        sys_guisock = accept(xsock, (struct sockaddr *) &server, 
            (socklen_t *)&len);
#ifdef OOPS
        close(xsock);
#endif
        if (sys_guisock < 0) sys_sockerror("accept");
        if (sys_verbose)
            fprintf(stderr, "... connected\n");

/* thread patch by ydegoyon@free.fr */
    // launch update thread
    if ( pthread_attr_init( &sys_updater_attr ) < 0 ) {
       post( "pd_startgui : could not launch update thread" );
       perror( "pthread_attr_init" );
       exit(-1);
    }
    if ( pthread_attr_setdetachstate( &sys_updater_attr, PTHREAD_CREATE_DETACHED ) < 0 ) {
       post( "pd_startgui : could not launch update thread" );
       perror( "pthread_attr_setdetachstate" );
       exit(-1);
    }
    if ( pthread_create( &sys_updater, &sys_updater_attr, sys_dosend, NULL ) < 0 ) {
       post( "pd_startgui : could not launch update thread" );
       perror( "pthread_create" );
       exit (-1);
    }
    else
    {
       post( "pd_startgui : updater thread %d launched", (int)sys_updater );
    }
    // no error return code
    pthread_mutex_init( &sys_update_mutex, NULL );
/* end thread patch by ydegoyon@free.fr */

    }
    if (!sys_nogui)
    {
      char buf[256], buf2[256];
         sys_socketreceiver = socketreceiver_new(0, 0, 0, 0);
         sys_addpollfn(sys_guisock, (t_fdpollfn)socketreceiver_read,
             sys_socketreceiver);

            /* here is where we start the pinging. */
#if defined(__linux__) || defined(IRIX)
         if (sys_hipriority)
             sys_gui("pdtk_watchdog\n");
#endif
         sys_get_audio_apis(buf);
         sys_get_midi_apis(buf2);
         sys_vgui("pdtk_pd_startup {%s} %s %s {%s}\n", pd_version, buf, buf2,
                                  sys_font); 
    }
    return (0);

}

extern void sys_exit(void);

/* This is called when something bad has happened, like a segfault.
Call glob_quit() below to exit cleanly.
LATER try to save dirty documents even in the bad case. */
void sys_bail(int n)
{
    static int reentered = 0;
    if (!reentered)
    {
        reentered = 1;
#ifndef __linux__  /* sys_close_audio() hangs if you're in a signal? */
        fprintf(stderr, "closing audio...\n");
        sys_close_audio();
        fprintf(stderr, "closing MIDI...\n");
        sys_close_midi();
        fprintf(stderr, "... done.\n");
#endif
        exit(n);
    }
    else _exit(1);
}

void glob_quit(void *dummy)
{
    sys_vgui("exit\n");
    if (!sys_nogui)
    {
        close(sys_guisock);
        sys_rmpollfn(sys_guisock);
    }
    sys_bail(0); 
}

