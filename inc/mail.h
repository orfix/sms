#ifndef  H_MAIL_MO_1273602224
#define  H_MAIL_MO_1273602224

#include  <sqlite3.h>

#define  DBFILENAME 	"emailDB.db"
#define  OBJECTMAXLEN 	128
#define  LOGINMAXLEN 	32
#define  LOGINMINLEN 	5
#define  PASSMINLEN 	5
#define  DELIMITER 		':'
#define  QUERYLEN		512
#define  HEADERSPP		5
#define  SIZEFIELD		11		/* 4294967296 + ':' */

//#define  ISLOGGED()     !!!!!!!!!!


typedef struct request
{
	const char *type;
	size_t argc;
	int (*action)(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]); /* DB connection */
}Request;

int loginf	 (sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]); 
int logoutf	 (sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]); 
int headsf	 (sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]); 
int readf	 (sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]); 
int deletef	 (sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]); 
int sendf	 (sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]); 
int registerf(sqlite3 *conn, int sd, char users[][LOGINMAXLEN], char *argv[]); 

int answer(sqlite3 *conn, int sd, char users[][LOGINMAXLEN]);
enum requestType getRequestType(const char *str);
int readArgs(int sd, char *argv[], size_t n);
void freeArgs(char *argv[], size_t n);
char *addField(char **dst, const unsigned char *field);

#endif  /* Guard */
