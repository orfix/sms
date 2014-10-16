/* /!\ DICLAIMER: this is a quick and dirty client app !! just for testing purposes */
#include  "wsock.h"
#include  "io.h"
#include  "util.h"
#include  "common.h"
#include  "net.h"
#include  <unistd.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <string.h>
#include  <ctype.h>

#define  PROGNAME	"SMC"
#define  VERSION	"0.1"

typedef struct requests {
	char type[32];
	//void (*action)(int sd, char argv[]);
	int (*action)(int sd, char argv[]);
}requests;

static int loginf(int sd, char argv[]);
static int logoutf(int sd, char argv[]);
static int headsf(int sd, char argv[]);
static int readf(int sd, char argv[]);
static int deletef(int sd, char argv[]);
static int sendf(int sd, char argv[]);
static int registerf(int sd, char argv[]);
static int help(int sd, char argv[]);

//static enum requestType getType(char *buf, char **end, requests request[]);
static enum requestType getType(char *word, requests request[]);
static int openTheConnection(char *argv[]);
//static char* getArg(char *str, char **end);
static char* getArg(char **s);
static int printReply(int sd);
static int sendRequest(int sd, char req[], char argv[], int n);

/* TODO	use real command options (short long optional) */
/* TODO	show error message when cannot connect to server */
/* TODO	show startup message + connected to blablabla stuff */
/* TODO help command */

static requests request[] = {
	{"LOGIN", 	loginf	},
	{"LOGOUT",	logoutf	},
	{"HEADS", 	headsf	},
	{"READ", 	readf	},
	{"DELETE", 	deletef	},
	{"SEND", 	sendf	},
	{"REGISTER",registerf},
	{"HELP", 	help	},
	{"", 		NULL	}
};

int main(int argc, char *argv[] )
{
	int sd;
	char exit = 0;
	char buf[BUFSIZE];
	char *start;
	enum requestType type; 

	if(argc < 4) {
		printf("Usage %s <ipv4 | ipv6> <host> <port>\n", argv[0]);
		return 1;
	}
	if( (sd=openTheConnection(argv)) == -1) {
		printf("Failed to connect to `%s' on port %s\n", argv[2], argv[3]);
		return 2;
	}

	puts("-- Copyright (C) 2010 Mounir Orfi <mounir.orfi@gmail.com>");
	puts("-- Welcome to "PROGNAME" version " VERSION);
	do {
		printf("mail> "); fflush(stdout);
		fgets(buf, BUFSIZE, stdin);	
		start = ltrim(buf);

		if(*start != '\0') {
			char *word = getArg(&start);

			//type = getType(start, &end, request);
			type = getType(word, request);

			if(start != NULL) {
				if(type != UNKNOWN) {
					/* read args */
					switch (request[type].action(sd, start)) {
						case 0 	: exit = 1; break;	/* connection closed by the server or fatal error */
						case -1 : puts("Request not sent, due to an error");
								  break;
						case -2 : printf("type `help %s' for more help\n", word);
								  break;
						case -3 : puts("Error while reading server reply");
								  break;
						default : break;
					}
					//} else if(strcmpi(word, "EXIT") == 0) {
			} else {
				printf("`%s' wrong command, see help\n", word);
			}
			}
		}
	}while( !exit );
	if(sd != -1) close(sd);

	return 0;
}

static enum requestType getType(char *word, requests request[])
{
	size_t i;

	for( i=0; request[i].action != NULL; i++) {
		if(strcmpi(word, request[i].type) == 0) {
			return i;
		}
	}

	return UNKNOWN;
}
#if 0
static enum requestType getType(char *buf, char **end, requests request[])
{
	size_t i;

	while(!isspace(**end)) {
		(*end)++;
	}
	**end++ = 0;

	for( i=0; request[i].action != NULL; i++) {
		if(strcmpi(buf, request[i].type) == 0) {
			return i;
		}
	}

	return UNKNOWN;
}
#endif

static int openTheConnection(char *argv[])
{
	struct addrinfo hints = {0};
	struct addrinfo *servinfo;
	struct addrinfo *p;
	int sd;

	/* defaults to ipv4 */
	hints.ai_family = strcmpi(argv[1], "ipv6")==0 ? AF_INET6 : AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	Getaddrinfo(argv[2], argv[3], &hints, &servinfo);

	for( p=servinfo; p!=NULL; p=p->ai_next) {
		sd = Socket(p->ai_family, p->ai_socktype, p->ai_protocol);
		if(sd == -1) continue;
		if(Connect(sd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sd);
			sd = -1;
			continue;
		} else {
			break;
		}
	}
	freeaddrinfo(servinfo);

	return sd;
}

static char* getArg(char **s)
{
	char *word = ltrim(*s);

	*s = word;
	if(*s != NULL && **s!='\0') {
		if(**s != '"') {
			while(**s!='\0' && !isspace(**s)) (*s)++;
		} else {
			word = ++(*s); /* skip the " */
			while(**s!='\0' && **s!='"') (*s)++;
		}

		if(**s != '\0') {
			**s = 0;
			(*s)++;
		} else {
			*s = NULL;
		}
	}

	return word;
}

#if 0
static char* getArg(char *str, char **end)
{
	*end = str;
	while(!isspace(**end)) {
		(*end)++;
	}
	**end++ = 0;
	return str;
}
#endif

static int printReply(int sd)
{
	int status;
	char *buf = NULL;

	if( (status=readField(sd, &buf, ':')) > 0) {
		printf("%c", strcmpi("OK", buf)==0 ? '+' : '-');
		free(buf);
		if((status=readField(sd, &buf, ':')) > 0) {
			puts(buf);
		}
		free(buf);
	}
	if(status < 0) status = -3;

	return status;
}

static int sendRequest(int sd, char req[], char argv[], int n) 
{	/* Make an assumption that the user will never input more than BUFSIZE :P */
	int status = -2;
	char *arg;
	size_t size = 1;	/* just to get in the loop */
	char buf[BUFSIZE] = {0};
	char miniBuf[BUFSIZE] = {0};

	snprintf(buf, BUFSIZE, "%d:%s", strlen(req), req);
	while(n-->0 && size>0) {
		arg = getArg(&argv);
		size = strlen(arg);
		//	printf("[%s] %d\n", arg, size);
		snprintf(miniBuf, BUFSIZE, "%d:%s", size, arg);
		strcat(buf, miniBuf);
	}

	if(size > 0) {
		//	printf("%d bytes sent:	%s\n", strlen(buf), buf);
		status = Send(sd, buf, strlen(buf), 0);
	}

	return status;
}


static int loginf(int sd, char argv[]) 
{
	int status = sendRequest(sd, "LOGIN", argv, 2);

	if(status > 0) {
		status = printReply(sd);
	}

	return status;
}

static int logoutf(int sd, char argv[])
{
	int status = sendRequest(sd, "LOGOUT", argv, 0);

	if(status > 0) {
		printReply(sd);
		status = 0;	/* exit */
	}

	return status;
}

static int headsf(int sd, char argv[])
{
	int status = sendRequest(sd, "HEADS", argv, 1);

	if(status > 0) {
		char *buf= NULL;

		if( (status=readField(sd, &buf, ':')) > 0 && strcmpi("OK", buf)==0 ) {
			char *id = NULL;
			char *from = NULL;
			char *subject = NULL;
			char *date = NULL;
			int n;

			free(buf);
			if( (status=readField(sd, &buf, ':')) > 0) {
				n = strtol(buf, NULL, 10);
				free(buf);

				printf("%-3s\t%-15s\t%-20s\t%s\t\n", "ID", "FROM", "DATE", "SUBJECT");
				puts("--------------------------------------------------------------------------------");
				while(n-- && status>0) {
					if( (status=readField(sd, &id, ':')) > 0 &&
							(status=readField(sd, &from, ':')) > 0 &&
							(status=readField(sd, &date, ':')) > 0 &&
							(status=readField(sd, &subject, ':')) > 0 ) {

						printf("%-3s\t%-15s\t%-20s\t%s\t\n", id, from, date, subject);
						free(id);
						free(from);
						free(date);
						free(subject);
					}
				}
				puts("");
			}
		} else { /* print error */
			free(buf);
			if((status=readField(sd, &buf, ':')) > 0) {
				printf("-%s\n", buf);
			}
			free(buf);
		}

		if(status < 0) status = -3;
	}

	return status;
}

static int readf(int sd, char argv[])
{
	int status = sendRequest(sd, "READ", argv, 1);

	if(status > 0) {
		char *from = NULL;
		char *subject = NULL;
		char *text = NULL;

		if( (status=readField(sd, &text, ':')) > 0 && strcmpi("OK", text)==0 ) {
			free(text);

			if( (status=readField(sd, &from, ':')) > 0 &&
					(status=readField(sd, &subject, ':')) > 0 &&
					(status=readField(sd, &text, ':')) > 0 ) {
				printf("\nFROM   : %s\n", from);
				printf("SUBJECT: %s\n", subject);
				printf("\n%s\n\n", text);
				free(from);
				free(subject);
				free(text);
			}
		} else { /* print error */
			free(text);
			if((status=readField(sd, &text, ':')) > 0) {
				printf("-%s\n", text);
			}
			free(text);
		}

		if(status < 0) status = -3;
	}

	return status;
}

static int deletef(int sd, char argv[])
{
	int status = sendRequest(sd, "DELETE", argv, 1);

	if(status > 0) {
		status = printReply(sd);
	}

	return status;
}

static int sendf(int sd, char argv[])
{
	int status = -2;
	char *to = getArg(&argv);
	char *subject = getArg(&argv);
	char buf[BUFSIZE] = {0};
	size_t bufSize = sizeof buf;
	char *line = buf;
	char *eol;

	if(*to!='\0' && *subject!='\0') {
		status = -1;
		puts("-- Your message should end with a dot `.' in a new line");
		while(fgets(line, bufSize, stdin) != NULL && strcmp(line, ".\n")) {
			bufSize -= strlen(line);
			eol = strrchr(line, '\n');
			if(eol != NULL) {
				line = eol +1;	/* eol+1 == \0 */
			} else {
				line += strlen(line);
				fpurge(stdin);
				break;	/* message was too long, cut it */
			}
		}
		*line = '\0';
		{
			char *toSend = NULL;
			size_t toLen = strlen(to);
			size_t subjectLen = strlen(subject);
			size_t sendSize = 6+11+toLen+11+subjectLen+11+sizeof buf -bufSize;
			/* 6 is for strlen("4:send") and 11 if for strlen("<length>:") */

			if( (toSend = malloc(sendSize)) !=NULL) {
				snprintf(toSend, sendSize, "4:SEND%d:%s%d:%s%d:%s",
						toLen, to, subjectLen, subject, strlen(buf), buf);
				status = Send(sd, toSend, strlen(toSend), 0);
				free(toSend);
				if(status > 0) {
					status = printReply(sd);
				}
			}
		}
	}
	return status;
}

static int registerf(int sd, char argv[])
{
	int status = sendRequest(sd, "REGISTER", argv, 3);

	if(status > 0) {
		status = printReply(sd);
	}

	return status;
}

static int help(int sd, char argv[]) 
{
	char *cmd = getArg(&argv);
	enum requestType type = getType(cmd, request);

	puts("");
	switch (type) {
		case LOGIN	: puts("Usage      : LOGIN <pseudonym> <password>\n"
						   "Description: Log in to your account\n"
						   "             You must be registered.");
					  break;
		case LOGOUT	: puts("Usage      : LOGOUT\n"
						   "Description: Log out from your account and close the connection.");
					  break;
		case HEADS	: puts("Usage      : HEADS <page number>\n"
						   "Description: Get the mail headers for the page requested.\n"
						   "             Page 0 is the most recent one.");
					  break;
		case READ	: puts("Usage      : READ <mail id>\n"
						   "Description: Retreive a mail.");
					  break;
		case DELETE	: puts("Usage      : DELETE <mail id>\n"
						   "Description: Delete a mail from your mailbox.");
					  break;
		case SEND	: puts("Usage      : SEND <recipient> <subject> '\\n' <message> '\\n' '.'\n"
						   "Description: Send a mail to another registered member.");
					  break;
		case REGISTER:puts("Usage      : REGISTER <full name> <pseudonym> <password>\n"
						   "Description: Become a registered member.");
					  break;
		default 	:
					  puts("Available commands:");
					  puts("HELP\t\tShow this help message");
					  puts("LOGIN\t\tLog in to your account");
					  puts("LOGOUT\t\tClose the connection");
					  puts("HEADS\t\tGet the mail headers");
					  puts("READ\t\tRetreive a mail");
					  puts("DELETE\t\tDelete a mail");
					  puts("SEND\t\tSend a mail to a registered member");
					  puts("REGISTER\tBecome a registered member");
					  break;
	}
	puts("");
	(void)sd;

	return 1;
}
