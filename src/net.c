#include  <stdio.h>
#include  <stdlib.h>
#include  <arpa/inet.h>
#include  "net.h"

unsigned short int get_port(struct sockaddr *saddr) 
{
	if(saddr->sa_family == AF_INET) {
		return ((SAI*)saddr)->sin_port;
	} else {
		return ((SAI6*)saddr)->sin6_port;
	}
}

void* get_in_addr(struct sockaddr *saddr) 
{
	if(saddr->sa_family == AF_INET) {
		return &((SAI*)saddr)->sin_addr.s_addr;
	} else {
		return &((SAI6*)saddr)->sin6_addr.s6_addr;
	}
}
