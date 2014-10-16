#include  <stdlib.h>
#include  <stdio.h>
#include  <syslog.h>
#include  <string.h>
#include  <errno.h>
#include  <unistd.h>
#include  <arpa/inet.h>
#include  "wsock.h"
#include  "net.h"
#include  <signal.h>

extern sig_atomic_t end;

void Getaddrinfo(const char *node, const char *service,
				const struct addrinfo *hints,
				struct addrinfo **res) {
	int status;

	if( (status=getaddrinfo(node, service, hints, res)!=0)) {
		if(status==EAI_SYSTEM) {
			syslog(LOG_ERR, "getaddrinfo: %s",strerror(errno));
		} else {
			syslog(LOG_ERR, "getaddrinfo: %s",gai_strerror(status));
		}
		exit(EXIT_FAILURE);
	}
}

int Socket(int domain, int type, int protocol)
{
	int s = socket(domain, type, protocol);

	if(s==-1) {
		syslog(LOG_ERR, "socket: %s",strerror(errno));
	}
	return s;
}

int Select(int nsd, fd_set *readsd, fd_set *writesd,
		   fd_set *exceptsd, struct timeval *timeout)
{
	int ret;
restart_select:
	if( (ret=select(nsd, readsd, writesd, exceptsd, timeout))==-1) {
		if(errno==EINTR) { /* in case it's interrupted by a signal handler */
			if(!end) { /* if the signal was not a SIGTERM */
				goto restart_select;
			}
			ret = 0;
		} else {
			syslog(LOG_ERR, "select: %s",strerror(errno));
			exit(EXIT_FAILURE);
		}
	}
	return ret;
}
void Listen(int sockfd, int backlog)
{
	if(listen(sockfd, backlog)==-1) {
		syslog(LOG_ERR, "listen: %s",strerror(errno));
		exit(EXIT_FAILURE);
	}
}

/*
void Setsockopt(int s, int level, int optname,
				const void *optval, socklen_t optlen)
{
	if(setsockopt(s, level, optname, optval, optlen)==-1) {
		syslog(LOG_ERR, "setsockopt: %s",strerror(errno));
		exit(EXIT_FAILURE);
	}
}*/

void Bind(int *listener, struct addrinfo *srvinfo)
{
	struct addrinfo *p;

	for( p=srvinfo; p!=NULL; p=p->ai_next) {
		*listener = Socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(*listener==-1) {
			continue; /* try next addresse */
		}
		if(bind(*listener, p->ai_addr, p->ai_addrlen)==-1) {
			syslog(LOG_ERR, "bind: %s",strerror(errno));
			close(*listener);
			continue; /* try next addresse */
		}
		break; 
	}

	if(p==NULL) {
		syslog(LOG_ERR, "no valid socket for binding");
		exit(EXIT_FAILURE);
	} else {
		char addrstr[INET6_ADDRSTRLEN];

		inet_ntop(p->ai_family, get_in_addr(p->ai_addr), addrstr, sizeof addrstr);
		syslog(LOG_INFO, "bind: succesffuly bounded to %s - %d on socket %d", addrstr,
			   ntohs(get_port(p->ai_addr)), *listener);
	}
	freeaddrinfo(srvinfo);
}

int Accept(int listener)
{
	struct sockaddr_storage remoteaddr;
	socklen_t addrlen = sizeof remoteaddr;
	int newsd;

restart_accept:
	newsd = accept(listener, (SA*)&remoteaddr, &addrlen);	
	if(newsd==-1) {
		if(errno==EINTR) { /* in case it's interrupted by a signal handler */
			goto restart_accept;
		}
		syslog(LOG_ERR, "accept: %s",strerror(errno));
	} else {
		char addrstr[INET6_ADDRSTRLEN];
		struct timeval timeout = {0, TIMEOUT}; /* 100 ms */

		inet_ntop(remoteaddr.ss_family, get_in_addr((SA*)&remoteaddr),
				  addrstr, sizeof addrstr);
		syslog(LOG_INFO, "accept: %s - %d connected on socket %d", addrstr,
			   ntohs(get_port((SA*)&remoteaddr)), newsd);
		setsockopt(newsd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
	}
	
	return newsd;
}

ssize_t Recv(int fd, void *buf, size_t len, int flags)
{
	ssize_t ret;
restart_recv:
	if( (ret=recv(fd, buf, len, flags))==-1) {
		if(errno==EINTR) { /* in case it's interrupted by a signal handler */
			goto restart_recv;
		}
		syslog(LOG_ERR, "recv: %s",strerror(errno));
	} 
	
	return ret;
}

ssize_t Send(int fd, const void *buf, size_t len, int flags) 
{
	ssize_t ret;
restart_send:
	if( (ret=send(fd, buf, len, flags))==-1) {
		if(errno==EINTR) { /* in case it's interrupted by a signal handler */
			goto restart_send;
		}
		syslog(LOG_ERR, "send: %s",strerror(errno));
	} 
	
	return ret;
}

int Connect(int sockfd, const struct sockaddr *serv_addr, socklen_t addrlen)
{
	ssize_t ret;
restart_connect:
	if( (ret=connect(sockfd, serv_addr, addrlen))==-1) {
		if(errno==EINTR) { /* in case it's interrupted by a signal handler */
			goto restart_connect;
		}
		syslog(LOG_ERR, "connect: %s",strerror(errno));
	} 
	
	return ret;
}
