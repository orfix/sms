#ifndef  H_ERR_MO_1273876654
#define  H_ERR_MO_1273876654

#define  ISFATAL(err)	(err>ERR_FATAL && err<=0)
#define  SHOULDREPLY(err)	(err<ERR_NOREPLY)

enum errors {
	ERR_NONE		= 1, 	/* OK */
	ERR_CLOSE 		= 0,	/* C: Connection closed by the client */
	ERR_NOREPLY		= -1,

	ERR_READ 		= -2,	/* S: Read error */
	ERR_DELIM 		= -3,	/* C: No delimiter found */
	ERR_CONV 		= -4,	/* C: Convertion error */
	ERR_LOGOUT		= -5,	/* -- not an error */
	ERR_FATAL		= -6,

	ERR_MALLOC 		= -7,	/* S: Memory allocation error */
	ERR_LOGIN 		= -8,	/* C: Invalid login/password combination */
	ERR_QUERY		= -9,	/* S: Query not compiled */

	ERR_REQTYPE 	= -10,	/* C: Invalid request type */
	ERR_ARGS 		= -11,	/* C: Invalid arguments */
	ERR_REGISTERED 	= -12,	/* C: Already registered */
	ERR_NOTLOGGED 	= -13,	/* C: Not logged */
	ERR_MAILID 		= -14,	/* C: Invalid email ID */
	ERR_PAGE 		= -15,	/* C: Invalid page */
	ERR_UNREGISTERED= -16,	/* C: Invalid user */


};

#endif  /* Guard */
