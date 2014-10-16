#include  "wsock.h"
#include  "err.h"
#include  "util.h"
#include  "io.h"
#include  "mail.h"
#include  "common.h"
#include  <syslog.h>
#include  <string.h>
#include  <stdio.h>
#include  <stdlib.h>
#include  <unistd.h>
#include  <sqlite3.h>
#include  <errno.h>
#include  <limits.h>

/* TODO: test the snprintf return value */
/* TODO: change the answer() function name */
/* TODO: create a generic answer function answer(type, message) -> "size:typesize:message" */
#define  ISLOGGED(sd, users)	(users[sd][0]!='\0')

/* /!\ Do not change the order of these messages before checking `enum errors' */
static char *errMsgs[] = {
	"",
	"",
	"5:ERROR29: Failed to read, retry please",
	"5:ERROR19: No delimiter found",
	"5:ERROR37: Convertion failed, invalid data size",

	"2:OK5: Bye!",
	"",

	"5:ERROR26: Memory allocation failure",
	"5:ERROR35: Invalid login/password combination",
	"5:ERROR19: Query not compiled",
	"5:ERROR21: Invalid request type",
	"5:ERROR18: Invalid arguments",
	"5:ERROR25: Login already registered",
	"5:ERROR12: Login first",
	"5:ERROR16: Invalid mail ID",
	"5:ERROR13: Invalid page",
	"5:ERROR36: Maybe the recipient is unregistered",

#if 0
	"",
	"0:",
	"0:",
	"5:ERROR30: Connection closed prematuraly", /* to remove */
	"5:ERROR?: ",
#endif
};

static Request requests[] = {
	{"LOGIN",	2, 	loginf	 },
	{"LOGOUT",	0, 	logoutf	 },
	{"HEADS",	1, 	headsf	 },
	{"READ",	1, 	readf	 },
	{"DELETE",	1, 	deletef	 },
	{"SEND",	3, 	sendf	 },
	{"REGISTER",3, 	registerf}
};
/* >0	a socket desciptor wich most be closed and the user logged out
 * =0	OK
 * <0	an error, the current socket descriptor most be closed and the user logged out
 */
int answer(sqlite3 *conn, int sd, char users[][LOGINMAXLEN]) 
{
	char *data = NULL;
	int ret;

	if( (ret=readField(sd, &data, DELIMITER)) > 0) {
		enum requestType type = getRequestType(data);

		if(type == UNKNOWN) {
			logit(LOG_ERR, sd, "Unknown command type `%s'", data);
			ret = ERR_REQTYPE;
		} else {
			/* TODO: use dynamic memory allocation based on requests[type].argc */
			char *argv[3] = {NULL};

			if((ret=readArgs(sd, argv, requests[type].argc))>0) {
				ret = requests[type].action(conn, sd, users, argv);
			}
			freeArgs(argv, requests[type].argc);
		}
		free(data);
#if 0
	} else if(ret==ERR_CLOSE || ret==ERR_READ) { /* -1 == ERR_MALLOC and erreur READ */
		ret = ERR_CLOSE;
#endif
	}
	if(SHOULDREPLY(ret)) { /* send an error message */
		Send(sd, errMsgs[-ret], strlen(errMsgs[-ret]), 0);
	}
	
	return ret;
}

enum requestType getRequestType(const char *str)
{
	size_t i = 0;

	while(i < NELEM(requests)) {
		if(strcmpi(str, requests[i].type)==0) {
			return i;
		}
		i++;
	}
	return UNKNOWN;
}

int readArgs(int sd, char *argv[], size_t n)
{
	size_t i = 0;
	int ret = 1;

	while( i<n && ret>0 ) {
		ret = readField(sd, &argv[i], DELIMITER);
		i++;
	}
	
	return ret;
}

void freeArgs(char *argv[], size_t n) {
	size_t i;

	for( i=0; i<n; i++) {
		if(argv[i]!=NULL) {
			free(argv[i]);
		}
	}
}

char *addField(char **dst, const unsigned char *field) 
{
	size_t dlen = (*dst==NULL) ? 0 : strlen(*dst);
	size_t flen = strlen((const char*)field);
	char *buf = malloc( 11 + 1 + flen + 1 ); /* 2:OK\0 */

	if(buf != NULL) {
		char *tmp;

		sprintf(buf, "%u:%s", flen, field);
		tmp = realloc(*dst, dlen + strlen(buf) + 1);

		if(tmp != NULL) {
			if(dlen == 0) {
				*tmp = '\0';
			}
			*dst = tmp;
			strncat(*dst, buf, strlen(buf)+1);
		}
		free(buf);
	}
	return *dst;
}

#if 0
char *addField(char **dst, const char *field) 
{
	size_t dlen = (*dst==NULL) ? 0 : strlen(*dst);
	size_t flen = strlen(field);
	char *buf = malloc( 11 + 1 + flen + 1 ); /* 2:OK\0 */


	if(buf != NULL) {
		char *tmp = realloc(*dst, dlen + SIZEFIELD + 1 + flen + 1);

		if(tmp != NULL) {
			if(dlen == 0) {
				*tmp = '\0';
			}
			*dst = tmp;
			sprintf(buf, "%u:%s", flen, field);
			strncat(*dst, buf, strlen(buf));
		}
		free(buf);
	}
	return *dst;
}
#endif

int loginf(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]) 
{
	int ret = ERR_NONE;
	char query[QUERYLEN] = "SELECT fullName FROM user WHERE login=? AND password=?";
	sqlite3_stmt *statement = NULL;
	char *login = argv[0];
	char *password = argv[1];

	if( sqlite3_prepare_v2( conn, query, sizeof query, &statement, NULL)==SQLITE_OK &&
		sqlite3_bind_text(statement, 1, login, -1, SQLITE_STATIC)==SQLITE_OK &&
		sqlite3_bind_text(statement, 2, password, -1, SQLITE_STATIC)==SQLITE_OK ) {

		if( sqlite3_step(statement)==SQLITE_ROW) {
				char buf[16 +LOGINMAXLEN+1] = "2:OK??: Welcome ";
				int i;
			const char *fullName = (const char*)sqlite3_column_text(statement, 0);

			for( i=0; i<FD_SETSIZE; i++) {
				if(strcmp(users[i], login)==0) { /* already logged... */
#if 0	/* FIX: disconnect users logged out */
					if(sd!=i) ret = i; /* do not disconnect a user issuing duplicate LOGIN requests */
#else
					if(sd!=i) *users[i] = '\0';	/* log him out */
#endif
					logit(LOG_INFO, sd, "`%s' was already logged on socket %d", login, i);
					break;
				}
			}
			strcpy(users[sd], login);
			logit(LOG_INFO, sd, "`%s' logged in", login);
			snprintf(buf, sizeof buf, "2:OK%d: Welcome %s", 9+strlen(fullName), fullName);
			Send(sd, buf, strlen(buf), 0);
		} else { /* invalid login/password */
			logit(LOG_ERR, sd, "Invalid login/password combination (%s/%s)", login, password);
			ret = ERR_LOGIN;
		}
		sqlite3_finalize(statement);	
	} else { /* query not compiled */
		logit(LOG_ERR, sd, "Query not compiled: %s", sqlite3_errmsg(conn));
		ret = ERR_QUERY;
	}
	return ret;
}

int logoutf(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[])
{
	//logit(LOG_INFO, sd, "`%s' logged out", users[sd]);

	(void) conn;
	(void) sd;
	(void) users;
	(void) argv;
	return ERR_LOGOUT;
}

int headsf(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[])
{
	int ret = ERR_NONE;
	char *end;
	long int page = strtol(argv[0], &end, 10);
	sqlite3_stmt *statement = NULL;
	char query[QUERYLEN] = "";

	if(!ISLOGGED(sd, users)) return ERR_NOTLOGGED;

	if(*end!='\0' || page<0) return ERR_PAGE;
	if(errno==ERANGE && (page==LONG_MIN || page==LONG_MAX)) return ERR_PAGE;

	sprintf(query, "SELECT * FROM email WHERE mailto=? ORDER BY receptiondate DESC LIMIT %ld, %d"
			, page*HEADERSPP, HEADERSPP);

	if( sqlite3_prepare_v2( conn, query, sizeof query, &statement, NULL)==SQLITE_OK &&
			sqlite3_bind_text(statement, 1, users[sd], -1, SQLITE_STATIC)==SQLITE_OK ) {

		char *bigbuf = NULL;
		if( sqlite3_step(statement)==SQLITE_ROW) {
			char *buf = malloc(4*SIZEFIELD); /* 4294967296 ':' */
			char tmp[SIZEFIELD+4] = "2:OK";
			int count = 0;
			char scount[SIZEFIELD] = "";

			*buf = '\0';
			do {
				addField(&buf, sqlite3_column_text(statement, 0));
				addField(&buf, sqlite3_column_text(statement, 2));
				addField(&buf, sqlite3_column_text(statement, 1));
				addField(&buf, sqlite3_column_text(statement, 4));
				count++;
			} while( sqlite3_step(statement)==SQLITE_ROW);

			sprintf(scount, "%d", count);
			sprintf(tmp, "2:OK%d:%d", strlen(scount) ,count);
			sstrcat(&bigbuf, tmp);
			sstrcat(&bigbuf, buf);
			free(buf);

			Send(sd, bigbuf, strlen(bigbuf), 0);
			logit(LOG_INFO, sd, "Page `%ld' retreived by `%s'", page, users[sd]);
			free(bigbuf);
		} else { /* invalid page */
			logit(LOG_ERR, sd, "Invalid page %ld requested", page);
			ret = ERR_PAGE;
		}
		sqlite3_finalize(statement);	
	} else { /* query not compiled */
		logit(LOG_ERR, sd, "Query not compiled: %s", sqlite3_errmsg(conn));
		ret = ERR_QUERY;
	}

	return ret;
}

int readf(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[])
{
	int ret = ERR_NONE;
	char *mailID = argv[0];
	char query[QUERYLEN]= "SELECT * FROM email WHERE id=? AND mailto=?";
	sqlite3_stmt *statement = NULL;

	if(!ISLOGGED(sd, users)) return ERR_NOTLOGGED;

	if( sqlite3_prepare_v2( conn, query, sizeof query, &statement, NULL)==SQLITE_OK &&
			sqlite3_bind_text(statement, 1, mailID, -1, SQLITE_STATIC)==SQLITE_OK &&
			sqlite3_bind_text(statement, 2, users[sd], -1, SQLITE_STATIC)==SQLITE_OK ) {

		if( sqlite3_step(statement)==SQLITE_ROW) {
			char *buf; 
			const unsigned char *mailfrom = sqlite3_column_text(statement, 2);
			const unsigned char *object = sqlite3_column_text(statement, 4);
			const unsigned char *message = sqlite3_column_text(statement, 5);
			long int mailfromLen = strlen((const char*)mailfrom); 
			long int objectLen = strlen((const char*)object); 
			long int messageLen = strlen((const char*)message); 
			size_t sendSize;
			size_t bufsize = mailfromLen + objectLen + messageLen + 
				5 + 	/* == strlen("2:OK") + '\0' */
				SIZEFIELD*3;	/* == 4294967296 + ':' * 3 */

			buf = malloc(bufsize);

			if(buf != NULL) {
				logit(LOG_INFO, sd, "Mail `%s' retreived by `%s'", mailID, users[sd]);
				sendSize = snprintf(buf, bufsize, "2:OK%ld:%s%ld:%s%ld:%s"
						,mailfromLen,mailfrom
						,objectLen, object
						,messageLen, message);
				Send(sd, buf, sendSize, 0);
				free(buf);
			} else {
				ret = ERR_MALLOC;
			}
		} else { /* invalid email ID */
			logit(LOG_ERR, sd, "Invalid email ID %s", mailID);
			ret = ERR_MAILID;
		}
		sqlite3_finalize(statement);	
	} else { /* query not compiled */
		logit(LOG_ERR, sd, "Query not compiled: %s", sqlite3_errmsg(conn));
		ret = ERR_QUERY;
	}
	return ret;
}

int deletef(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[])
{
	int ret = ERR_NOTLOGGED;
	char *mailID = argv[0];

	if(ISLOGGED(sd, users)) {
		char buf[32] = "";
		char query[QUERYLEN] = "DELETE FROM email WHERE id=? AND mailto=?";
		sqlite3_stmt *statement = NULL;

		ret = ERR_NONE;
		if( sqlite3_prepare_v2( conn, query, sizeof query, &statement, NULL)==SQLITE_OK &&
				sqlite3_bind_text(statement, 1, mailID, -1, SQLITE_STATIC)==SQLITE_OK &&
				sqlite3_bind_text(statement, 2, users[sd], -1, SQLITE_STATIC)==SQLITE_OK ) {

			if( sqlite3_step(statement)==SQLITE_DONE) {
				if(sqlite3_changes(conn) > 0) {
					logit(LOG_INFO, sd, "Mail `%s' deleted", mailID);
					snprintf(buf, sizeof buf, "2:OK%d: Mail %s deleted", 14+strlen(mailID), mailID);
					Send(sd, buf, strlen(buf), 0);
				} else { /* invalid email ID */
					logit(LOG_ERR, sd, "Invalid email ID %s", mailID);
					ret = ERR_MAILID;
				}
			} else {
				logit(LOG_ERR, sd, "Query not compiled: %s", sqlite3_errmsg(conn));
				ret = ERR_QUERY;
			}
			sqlite3_finalize(statement);	
		} else { /* query not compiled */
			logit(LOG_ERR, sd, "Query not compiled: %s", sqlite3_errmsg(conn));
			ret = ERR_QUERY;
		}
	}
	return ret;
}

int sendf(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[])
{
	int ret = ERR_NOTLOGGED;
	char *mailto = argv[0];
	char *object = argv[1];
	char *message = argv[2];

	if(ISLOGGED(sd, users)) {
		char query[QUERYLEN] = "INSERT INTO email VALUES(NULL, datetime('now'), ?1, ?2, ?3, ?4)";
		sqlite3_stmt *statement = NULL;

		if(strlen(object)>OBJECTMAXLEN) {
			ret = ERR_ARGS;
		} else {
			ret = ERR_NONE;
			if( sqlite3_prepare_v2( conn, query, sizeof query, &statement, NULL)==SQLITE_OK) {
				if( sqlite3_bind_text(statement, 1, users[sd], -1, SQLITE_STATIC)==SQLITE_OK &&
						sqlite3_bind_text(statement, 2, mailto, -1, SQLITE_STATIC)==SQLITE_OK &&
						sqlite3_bind_text(statement, 3, object, -1, SQLITE_STATIC)==SQLITE_OK &&
						sqlite3_bind_text(statement, 4, message, -1, SQLITE_STATIC)==SQLITE_OK ) {

					char buf[] = "2:OK12: Mail sended";

					if( sqlite3_step(statement)==SQLITE_DONE) {
						//if(sqlite3_changes(conn) > 0) {
						logit(LOG_INFO, sd, "Mail `%s' sended", object);
						Send(sd, buf, strlen(buf), 0);
					} else { /* recipient not found */
						logit(LOG_ERR, sd, "Maybe `%s' is unregistered", mailto);
						ret = ERR_UNREGISTERED;
					}
				}
				sqlite3_finalize(statement);	
			} else { /* query not compiled */
				logit(LOG_ERR, sd, "Query not compiled: %s", sqlite3_errmsg(conn));
				ret = ERR_QUERY;
			}
		}
	}

	return ret;
}

int registerf(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[])
{
	int ret = ERR_NONE;
	char query[QUERYLEN] = "INSERT INTO user VALUES(?, ?, ?, datetime('now'))";
	sqlite3_stmt *statement = NULL;
	char *fullname = argv[0];
	char *login = argv[1];
	char *password = argv[2];
	size_t loginLen = strlen(login);
	size_t passLen = strlen(password);

	if(loginLen<=LOGINMAXLEN && loginLen>=LOGINMINLEN &&
			passLen>=PASSMINLEN && fullname[0]!='\0' && 
			strstr(login," ")==NULL) {
		if( sqlite3_prepare_v2( conn, query, sizeof query, &statement, NULL)==SQLITE_OK &&
				sqlite3_bind_text(statement, 1, login, -1, SQLITE_STATIC)==SQLITE_OK &&
				sqlite3_bind_text(statement, 2, password, -1, SQLITE_STATIC)==SQLITE_OK &&
				sqlite3_bind_text(statement, 3, fullname, -1, SQLITE_STATIC)==SQLITE_OK ) {

			char buf[] = "2:OK38: congratulation you are now registered";

			if( sqlite3_step(statement)==SQLITE_DONE) {
				logit(LOG_INFO, sd, "`%s' registered", login);
				Send(sd, buf, sizeof buf -1, 0);	/* -1 for the \0 */
			} else { /* login already registered */
				logit(LOG_ERR, sd, "`%s' already registered", login);
				ret = ERR_REGISTERED;
			}
			sqlite3_finalize(statement);	
		} else { /* query not compiled */
			logit(LOG_ERR, sd, "Query not compiled: %s", sqlite3_errmsg(conn));
			ret = ERR_QUERY;
		}
	} else {
		logit(LOG_ERR, sd, "Invalid parameters");
		ret = ERR_ARGS;
	}

	(void) sd;
	(void) users;
	return ret;
}
