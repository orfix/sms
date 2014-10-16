#ifndef  H_IO_MO_1273409198
#define  H_IO_MO_1273409198

#include  <sys/types.h>

ssize_t readn(int fd, void *buf, ssize_t len);
ssize_t readc(int fd, char *buf, size_t n, char delim);
ssize_t readField(int fd, char **data, char delim);

#endif  /* Guard */
