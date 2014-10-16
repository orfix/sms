#ifndef  H_COMMON_MO_1275772202
#define  H_COMMON_MO_1275772202

enum requestType {
	LOGIN,		/* login, password */
	LOGOUT,		/* ... */
	HEADS,		/* Page number */
	READ,		/* mail ID */
	DELETE,		/* mail ID */
	SEND,		/* recipient, object, message */
	REGISTER,	/* name, login, password */
};
#define UNKNOWN	1024	/* request type */


#endif  /* Guard */
