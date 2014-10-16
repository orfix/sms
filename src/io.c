#include  "io.h"
#include  "wsock.h"
#include  "err.h"
#include  <sys/types.h>
#include  <sys/socket.h>
#include  <stdlib.h>
#include  <errno.h>
#include  <unistd.h>
#include  <syslog.h>
#include  <string.h>
#include  <stdio.h>
#include  <limits.h>

/* TODO: Fix DOS attack vulnerability */

ssize_t readn(int fd, void *buf, ssize_t len)
{
	ssize_t n = 0;
	ssize_t total = 0;

	do {
		if( (n=Recv(fd, buf+total, len-total, 0))==-1) {
			syslog(LOG_ERR, "readn: %s",strerror(errno));
		}
	}while( n>0 && (total+=n)<len );
	/* > 0 		number of bytes read (always equal to len)
	 * == 0		peer closed the connection (incomplete)
	 * == -1	reading error
	 */
	return n<=0 ? n : total; 
}

ssize_t readc(int fd, char *buf, size_t n, char delim)
{
	char *p = buf;
	char *end = buf +n -1;
	int status = ERR_DELIM;
	ssize_t ret;

	/* read one byte at a time looking for the delimiter */	
	while( p<end && (status=Recv(fd, p, 1, 0)) > 0) {
		//printf("[%c]", *p);
		if(*p == delim) { /* found ! */
			//ret = p-buf +1;
			break;
		}
		p++;
	}
	if(p == end) {
		ret = ERR_DELIM; /* no delimiter found */
	} else if(*p == delim) {
		ret = p-buf +1; /* number of bytes read */
	} else {
		ret = status; /* recv() status (0 or -1) */
	}
	/* > 0 		number of bytes read
	 * == 0		peer closed the connection (incomplete)
	 * == -1	reading error
	 * == -2	no delimiter found
	 */

	return ret;
}

ssize_t readField(int fd, char **data, char delim)
{
	/* ==0 		peer closed the connection
	 * >0		number of bytes read
	 * -1		reading error
	 * -2		no delimiter found
	 * -3		convertion error
	 * -4		memory allocation error
	 */
	char buf[32];
	ssize_t ret;
	/* read size */
	*data = NULL;
	memset(buf, 0, sizeof buf); /* to get a valid C string */
	if( (ret=readc(fd, buf, sizeof buf -1, delim)) > 0) {
		char *end;
		long int size;

		/* TAKE OFF THE \0  */
#if 0
		if(*buf == '\0') {
			size = strtol(buf+1, &end, 10);
		} else {
#endif
			size = strtol(buf, &end, 10);
	//	}
		if(*end==delim && size>0) {
			if(errno==ERANGE && (size==LONG_MIN || size==LONG_MAX)) {
				ret = ERR_CONV; /* overflow or underflow error */
			} else { /* read data */
				if( (*data=malloc(size +1))==NULL) { /* +1 for \0 */
					size = ERR_MALLOC; /* memory allocation error */
				} else {
					if( (ret=readn(fd, *data, size)) > 0) {
						(*data)[size] = '\0';
						ret = size;
					}
				}
			}
		} else { /* conversion failed */
			ret = ERR_CONV;
		}
		//printf("data = %s\n", *data);
	}

	return ret;
}
