#define _POSIX_C_SOURCE 200112L
// posix standard with getaddrinfo

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>
// #include <netinet/in.h>

#include "funcs.c"
 
int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage error\n");
		return 1;
	}

	int i = create_sock_bind_send(argv[1]);
	if (i) {
		return i;
	}
	return 0;
}
