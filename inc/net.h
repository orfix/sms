#ifndef  H_NET_MO_1272409140
#define  H_NET_MO_1272409140

#define  LISTENQ        1024    /* backlog, 2nd argument to listen() */
#define  SRVPORT		"9876"
/* 5000  - 49152 : correct range to avoid conflicts
 * 0     - 1023  : well-known ports
 * 1024  - 5000  : traditional BSD ephemeral
 * 1024  - 49151 : IANA registred ports 
 * 49152 - 65535 : IANA dynamic or private ports (ephemeral)
 */

/* Miscellaneous constants */
#define  BUFSIZE        4096
#define  LINESIZE       2048

/* Shortening typecasts */
typedef struct sockaddr SA;
typedef struct sockaddr_in SAI;
typedef struct sockaddr_in6 SAI6;
typedef void t_handler(int);

void* get_in_addr(struct sockaddr *saddr);
unsigned short int get_port(struct sockaddr *saddr);

#endif  /* Guard */
