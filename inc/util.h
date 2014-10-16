#ifndef  H_UTIL_MO_1273608251
#define  H_UTIL_MO_1273608251

#include  <stddef.h>
#include  <stdio.h>

#define  NELEM(t)	(sizeof(t)/sizeof(*t))
/* #define  HAVE_STRCMPI */
/* #define  __FUNCTION__ as "UNKNOWN FUNCTION" if not present */
#define  logit(t, s, f, ...)	syslog(t, "%s: socket %d: " f, __FUNCTION__, s, ##__VA_ARGS__)
#define  fatal(fmt, ...) \
do{ \
	fprintf(stderr, fmt, __VA_ARGS__); \
	exit(EXIT_FAILURE); \
}while(0);

int strcmpi(const char *s1, const char *s2);
char *sstrcat(char **dst, const char *src);
char *ltrim(char *str);
int fpurge(FILE *fd);

#endif  /* Guard */
