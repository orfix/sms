#include  "io.h"
#include  "util.h"
#include  "net.h"
#include  "mail.h"
#include  "wsock.h"
#include  "err.h"
#include  <sys/types.h>
#include  <sys/stat.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <syslog.h>
#include  <unistd.h>
#include  <sqlite3.h>
#include  <signal.h>

/* TODO: Catch SIGTERM */
/* TODO: Catch SIGPIPE, handle EPIPE in Send  */
/* TODO: Rewrite it as a daemon */
/* TODO: send responses */

sig_atomic_t end = 0;

void closeAndLogout(int sd, fd_set *allsd, char users[][LOGINMAXLEN]);

void daemonize(void) 
{
	pid_t pid = fork();
	pid_t sid;

	if(pid > 0) {
		exit(EXIT_SUCCESS);
	} else if(pid < 0) {
		exit(EXIT_FAILURE);
	}
	umask(0);
	if( (sid=setsid()) < 0) {
		exit(EXIT_FAILURE);
	}
	/*
	if(chdir("/") < 0) {
		exit(EXIT_FAILURE);
	}
	*/
	/* close standard desciptors */
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	close(STDERR_FILENO);
	freopen("/dev/null", "w", stdout);
}


void quit(int signum) 
{
	end = 1;
	(void)signum;
}

int main(int argc, char *argv[])
{
	char users[FD_SETSIZE][LOGINMAXLEN] = {{""}};
	struct addrinfo hints = {0};
	struct addrinfo *srvinfo;
	int listener;
	fd_set readsd, allsd;
	int maxsd, nready, sd, newsd;
	int reuseAddr = 1;
	sqlite3 *connection = NULL;
	long int port;

	/* set up the SIGTERM handler */
	signal(SIGTERM, quit);
	/* Read command line arguments */
	if(argc>2) { 
		port = strtol(argv[1], NULL, 10);
		if(port > 65535 || port <0) {
			exit(EXIT_FAILURE);
		}
	}
	/* Open the database connection */
	if( sqlite3_open_v2(DBFILENAME, &connection, SQLITE_OPEN_READWRITE, NULL)!=SQLITE_OK) {
		//fprintf(stderr, "%s\n", sqlite3_errmsg(connection));	
		/* Always close the connection whether it was correctly openned or not */
		sqlite3_close(connection);
		exit(EXIT_FAILURE);
	}

	//daemonize();

	hints.ai_family   = AF_UNSPEC;  /* Allow IPv4 & IPv6 */
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags 	  = AI_PASSIVE; /* For wildcard IP address */
	Getaddrinfo(NULL, (argc==2)? argv[1] : SRVPORT, &hints, &srvinfo);

	Bind(&listener, srvinfo);
	/* to avoid the "addresse already in use" error message */
	setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof reuseAddr);
	Listen(listener, LISTENQ);
	maxsd = listener;
	FD_ZERO(&allsd);
	FD_SET(listener, &allsd);

	/* MAIN EVENT LOOP */
	while(!end) {
		readsd = allsd; /* update readsd (value-result) */
		nready = Select(maxsd+1, &readsd, NULL, NULL, NULL); /* blocking call */
		if(nready > 0) 
		{
			/* look for data to read */
			for( sd=0; sd<=maxsd ; sd++) {
				if(FD_ISSET(sd, &readsd)) {
					if(sd==listener) { /* handle a new connection */
						newsd = Accept(listener);
						if(newsd!=-1) {
							FD_SET(newsd, &allsd);
							/* TODO: keep track of the maxsd if the maxsd is closed */
							if(newsd > maxsd) {
								maxsd = newsd;
							}
						} 
					} else { /* handle data from a client */
						int err;
						puts("something happened...");

						if((err=answer(connection, sd, users)) <= 0) {
							/* something goes wrong */
							if(ISFATAL(err)) {
								closeAndLogout(sd, &allsd, users);
							}
#if 0
						} else if(err > 0) { /* the peer was already logged */
							closeAndLogout(err, &allsd, users);
#endif
						}
						fprintf(stderr,"error %d\n", err);
					}
					if(--nready <=0) {
						break; /* all file descriptors were handled */
					}
				}
			}
		}
	}
	sqlite3_close(connection);
	for( sd=0; sd<=maxsd ; sd++) {
		if(FD_ISSET(sd, &allsd)) {
			logit(LOG_INFO, sd, "connection closed");
			close(sd);
		}
	}

	return 0;
}

void closeAndLogout(int sd, fd_set *allsd, char users[][LOGINMAXLEN])
{
	close(sd);
	FD_CLR(sd, allsd);
	logit(LOG_INFO, sd, "`%s' logged out", users[sd]);
	users[sd][0] = '\0';
	logit(LOG_INFO, sd, "connection closed");
	//users[sd].sd = -1;
	//users[newsd].login[0] = '\0';
}

