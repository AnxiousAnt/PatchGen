/* Copyright (c) 2000 Miller Puckette.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in the Pd distribution.  */

/* the "pdsend" command.  This is a standalone program that forwards messages
from its standard input to Pd via the netsend/netreceive ("FUDI") protocol. */

#include <sys/types.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#ifdef UNIX
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#define SOCKET_ERROR -1
#else
#include <winsock.h>
#endif

void sockerror(char *s);
void my_closesocket(int fd);
#define BUFSIZE 4096

int main(int argc, char **argv)
{
    int portno, protocol;
    struct sockaddr_in server;
    struct hostent *hp;
    char *hostname;
    int nretry = 10;
#ifdef NT	/* added by olaf.matthes@gmx.de */
	SOCKET sockfd;
    short version = MAKEWORD(2, 0);
    WSADATA nobby;
	/* we need to activate the ws2_32.dll !!! */
    WSAStartup(version, &nobby);
#else
	int sockfd;
#endif
    if (argc < 2 || sscanf(argv[1], "%d", &portno) < 1 || portno <= 0)
    	goto usage;
    if (argc >= 3)
    	hostname = argv[2];
    else hostname = "127.0.0.1";
    if (argc >= 4)
    {
    	if (!strcmp(argv[3], "tcp"))
	    protocol = SOCK_STREAM;
	else if (!strcmp(argv[3], "udp"))
	    protocol = SOCK_DGRAM;
	else goto usage;
    }
    else protocol = SOCK_STREAM;

    sockfd = socket(AF_INET, protocol, 0);
#ifdef NT	/* added by olaf.matthes@gmx.de */
	if (sockfd == INVALID_SOCKET)
#else
    if (sockfd < 0)
#endif
    {
    	sockerror("socket()");
    	exit(1);
    }

    	/* connect socket using hostname provided in command line */
    server.sin_family = AF_INET;
    hp = gethostbyname(hostname);
    if (hp == 0)
    {
		fprintf(stderr, "%s: unknown host\n", hostname);
		closesocket(sockfd);
		exit(1);
    }
    memcpy((char *)&server.sin_addr, (char *)hp->h_addr, hp->h_length);

    	/* assign client port number */
    server.sin_port = htons((unsigned short)portno);

#if 0	/* try this again for 4.0; this crashed my RH 6.2 machine!) */

	/* try to connect.  */
    for (nretry = 0; nretry < (protocol == SOCK_STREAM ? 10 : 1); nretry++)
    
    {
    	if (nretry > 0)
	{
	    sleep (nretry < 5 ? 1 : 5);
	    fprintf(stderr, "retrying...");
	}
	if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) >= 0)
	    goto connected;
    	sockerror("connect");
    }
    closesocket(sockfd);
    exit(1);
connected: ;
#else
    /* try to connect.  */
    if (connect(sockfd, (struct sockaddr *) &server, sizeof (server)) < 0)
    {
    	sockerror("connect");
    	closesocket(sockfd);
    	exit(1);
    }
#endif
    	/* now loop reading stdin and sending  it to socket */
    while (1)
    {
    	char buf[BUFSIZE], *bp, nsent, nsend;
    	if (!fgets(buf, BUFSIZE, stdin))
	    break;
    	nsend = strlen(buf);
	for (bp = buf, nsent = 0; nsent < nsend;)
	{
	    int res = send(sockfd, buf, nsend-nsent, 0);
	    if (res < 0)
	    {
	    	sockerror("send");
		goto done;
	    }
    	    nsent += res;
	    bp += res;
	}
    }
done:
    if (ferror(stdin))
    	perror("stdin");
    exit (0);
usage:
    fprintf(stderr, "usage: pdsend <portnumber> [host] [udp|tcp]\n");
    fprintf(stderr, "(default is localhost and tcp)\n");
    exit(1);
}

void sockerror(char *s)
{
#ifdef NT
    int err = WSAGetLastError();
    if (err == 10054) return;
    else if (err == 10044)
    {
    	fprintf(stderr,
	    "Warning: you might not have TCP/IP \"networking\" turned on\n");
    }
#endif
#ifdef UNIX
    int err = errno;
#endif
    fprintf(stderr, "%s: %s (%d)\n", s, strerror(err), err);
}

void my_closesocket(int fd)
{
#ifdef UNIX
    close(fd);
#endif
#ifdef NT
    closesocket(fd);
#endif
}
