#ifndef  H_WSOCK_MO_1272409502
#define  H_WSOCK_MO_1272409502

#include  <sys/types.h>
#include  <sys/socket.h>
#include  <netdb.h>
#include  <sys/select.h>

#define  TIMEOUT	900000		/* 1 seconds */

void Getaddrinfo(const char *node, const char *service,
			const struct addrinfo *hints,
			struct addrinfo **res);
int Socket(int domain, int type, int protocol);
void Setsockopt(int sockfd, int level, int optname,
				const void *optval, socklen_t optlen);
int Select(int nfds, fd_set *readfds, fd_set *writefds,
		   fd_set *exceptfds, struct timeval *timeout);
void Listen(int sockfd, int backlog);	
void Bind(int *listener, struct addrinfo *srvinfo);
int Accept(int listener);
ssize_t Send(int fd, const void *buf, size_t len, int flags);
ssize_t Recv(int fd, void *buf, size_t len, int flags);
int Connect(int sockfd, const struct sockaddr *serv_addr,
			socklen_t addrlen);


#endif  /* Guard */
