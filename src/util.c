#include  "util.h"
#include  <ctype.h>
#include  <string.h>
#include  <stdlib.h>

#ifndef HAVE_STRCMPI
int strcmpi(const char *s1, const char *s2)
{
	for( ; *s1!='\0' && *s2!='\0'; s1++, s2++) {
		if(tolower(*s1) != tolower(*s2)) {
			break;
		}
	}
	return *s1 - *s2;
}
#endif

char *sstrcat(char **dst, const char *src)
{
	size_t dlen = (*dst==NULL) ? 0 : strlen(*dst);
	size_t slen = strlen(src);
	char *tmp = realloc(*dst, dlen + slen + 1);

	if(tmp != NULL) {
		if(dlen == 0) {
			*tmp = '\0';
		}
		*dst = tmp;
		strncat(*dst, src, slen);
	}

	return tmp;
}

char *ltrim(char *str)
{
	while( isspace(*str) ) str++;

	return str;
}

int fpurge(FILE *fd)
{
	int c;

	while((c=fgetc(fd))!='\n' && c!=EOF)
	{}
	return (c==EOF ? -1 : 0);
}
