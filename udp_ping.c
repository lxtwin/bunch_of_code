/*
 * This is a small but usefull program that can be used to
 * check to see if a host is up or down.
 *
 */

#include	<stdio.h>
#include	<errno.h>
#include	<signal.h>
#include	<sys/types.h>
#include	<sys/socket.h>
#include	<sys/time.h>
#include	<netinet/in.h>
#include	<netdb.h>

#define	ECHO	"echo" 	/* from /etc/services */
#define	PROTO	"udp" 	/* from /etc/services */

char *host;
char *prog;
extern errno;

static char Sccsid[] = "@(#) udp_ping.c                 Version 1.1, Date: 97/04/11 - Pkr";


int main(int argc, char** argv);
int ping(void);
int down(void);
int time_now(void);

int sock, sent, recvd;
struct sockaddr_in me, it;
struct servent *serv_name;

int main(int argc, char** argv)
{

struct servent *serv_name;
struct hostent* h;
char buf[255];
int  seq, time, i, namelen;
register size, delta;

	sent = 1;

	if (argc < 2) {
		printf("usage: %s host\n", argv[0]);
		exit(255);
	}
	prog = argv[0];
	host = argv[1];

	if (!(h = gethostbyname(argv[1]))) {
		fprintf(stderr, "gethostbyname failed to find host %s\n", host);
		exit(255);
	} /* else 
		printf("[gethostbyname: %s]\n", h->h_name); */

	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0) {
		perror("socket");
		exit(255);
	}

	if (!(serv_name = getservbyname(ECHO, PROTO))) {
		fprintf(stderr, "getservbyname failed to find %s/%s in /etc/services\n", 
			ECHO, PROTO);
		exit(5);
	} /* else
		printf("[getservbyname: %d]\n", serv_name->s_port); */

	bzero(&it, sizeof(it));
	it.sin_family = AF_INET;
	bcopy(h->h_addr, &it.sin_addr, h->h_length);
	it.sin_port = serv_name->s_port;

	namelen = sizeof(it);

#ifdef _HPUX_SOURCE
	signal(SIGALRM, down);	/* Could give a compile error on HP */
#else
	sigset(SIGALRM, down);
#endif
	alarm(10); /* works better 4 than 1 */
	ping();

	size = recvfrom(sock, buf, sizeof(buf), 0, &it, &namelen);
	if (size == -1  &&  errno == EINTR)
		perror("size");
	recvd++;
	sscanf(buf, "Paul's Sequence: %d, %d", &seq, &time);
	delta = time_now() - time;
	/* fprintf(stderr, "Recieved: [%s], in %.2f seconds\n",
	    buf, delta/1000.0); */

	printf("udp_ping: host \"%s\" is UP.\n", host);
	exit(0);
}

/*
 *	This bit sends out the packet
 */
int ping()
{

char buf[256];

	/* fprintf(stdout, "Sending: Paul's Sequence: %03d, %d\n", sent, time_now()); */
	sprintf(buf, "Paul's Sequence: %03d, %d", sent, time_now());
	if (sendto(sock, buf, strlen(buf) + 1, 0, &it, sizeof it) < 0) {
		perror("send failed");
		exit(6);
	}
	/* If we are here then the send worked */
	return(0);
}

/*
 *	This bit is called by the alarm, and if we get here it failed.
 */
int down()
{
	fprintf(stderr, "udp_ping: host \"%s\" is DOWN.\n", host);
	exit(4); /* could be a return HOST_DOWN */
}

int time_now()
{

struct timeval now;

	gettimeofday(&now, 0);
	return now.tv_sec * 1000 + now.tv_usec / 1000;
}
