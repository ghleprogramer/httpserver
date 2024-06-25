#define _POSIX_C_SOURCE 200112L
// posix standard with getaddrinfo

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
// #include <netinet/in.h>
 
#define STR_PORT_NUMBER "3490"

int main(int argc, char **argv)
{
	struct addrinfo hints, *srv_info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = PF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	
	int getaddrcheck = getaddrinfo(NULL, STR_PORT_NUMBER, &hints, &srv_info);
	if (getaddrcheck != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(getaddrcheck));
		return getaddrcheck;
	}

	int clnt_fd;
	clnt_fd = socket(srv_info->ai_family, srv_info->ai_socktype, srv_info->ai_protocol);
	if (clnt_fd == -1) {
		fprintf(stderr, "socket error: %s\n", strerror(errno));
		return errno;
	}

	connect(clnt_fd, srv_info->ai_addr, srv_info->ai_addrlen);

	FILE *f = fopen("/home/poggers/Desktop/recv", "wb");
	if (f == NULL) {
		fprintf(stderr, "fopen error: %s\n", strerror(errno));
		return errno;
	}
	uint32_t buff;
	
	while (recv(clnt_fd, &buff, sizeof(buff), 0)) {
		fwrite(&buff, sizeof(buff), 1, f);
	}

	freeaddrinfo(srv_info);
	// printf("nofail\n");
	return 0;
}
