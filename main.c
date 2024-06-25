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

#include "funcs.c"
 
#define LISTEN_QUEUE 10
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

	int srv_fd;
	srv_fd = socket(srv_info->ai_family, srv_info->ai_socktype, srv_info->ai_protocol);
	if (srv_fd == -1) {
		fprintf(stderr, "socket error: %s\n", strerror(errno));
		return errno;
	}

	if (bind(srv_fd, srv_info->ai_addr, srv_info->ai_addrlen) == -1) {
		fprintf(stderr, "bind error: %s\n", strerror(errno));
		return errno;
	}

	if (listen(srv_fd, LISTEN_QUEUE) == -1) {
		fprintf(stderr, "listen error: %s\n", strerror(errno));
		return errno;
	}

	int clnt_fd;
	struct sockaddr_storage clnt_addr;
	socklen_t clnt_addr_len = sizeof(clnt_addr);

	clnt_fd = accept(srv_fd, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
	if (clnt_fd == -1) {
		fprintf(stderr, "accept error: %s\n", strerror(errno));
		return errno;
	}
	
	FILE *msg = fopen("/home/poggers/Desktop/conecct.txt", "rb");
	if (msg == NULL) {
		fprintf(stderr, "fopen error: %s\n", strerror(errno));
		return errno;
	}
	uint32_t msgbuff;
	int bytes_sent;

	while (fread(&msgbuff, sizeof(msgbuff), 1, msg)) {
		bytes_sent = 0;
		do
		{
			bytes_sent = send(clnt_fd, &msgbuff, sizeof(msgbuff), 0);
			if (bytes_sent == -1) {
				fprintf(stderr, "send error: %s\n", strerror(errno));
				return errno;
			}
		} while (bytes_sent < sizeof(msgbuff));
		
	}

	freeaddrinfo(srv_info);
	// printf("nofail\n");
	return 0;
}
