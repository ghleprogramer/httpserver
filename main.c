#define _POSIX_C_SOURCE 200112L
// posix standard with getaddrinfo

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "funcs.c"

#define LISTEN_QUEUE 10
#define SERVICE "http" // "3490" "8080"
#define NAME_MAX_LENGTH 50
#define HTTP_RECV_SIZE 512
#define SEND_BUFFER_SIZE 1000*10 // 10KB

int main(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr, "usage error\n");
		return 1;
	}

	int srv_fd;
	int create_srv_ckeck;
	if ((create_srv_ckeck = create_bind_stream_sock(&srv_fd))) {
		return create_srv_ckeck;
	}

	if (listen(srv_fd, LISTEN_QUEUE) == -1) {
		fprintf(stderr, "listen error: %s\n", strerror(errno));
		return errno;
	}

	int clnt_fd;
	struct sockaddr_storage clnt_addr;
	socklen_t clnt_addr_len = sizeof(clnt_addr);

	while (1) {		
		clnt_fd = accept(srv_fd, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
		if (clnt_fd == -1) {
			fprintf(stderr, "accept error: %s\n", strerror(errno));
			return errno;
		}

		uint8_t recvbuffer[HTTP_RECV_SIZE];
		memset(recvbuffer, 0, sizeof(recvbuffer));
		if (recv(clnt_fd, recvbuffer, sizeof(recvbuffer), 0) == -1) {
			fprintf(stderr, "recv error: %s\n", strerror(errno));
			return errno;
		}
		printf("%s", (char *)recvbuffer);

		char ipstr[INET6_ADDRSTRLEN];
		char hostname[NAME_MAX_LENGTH];
		char servicename[NAME_MAX_LENGTH];

		const char* ntopcheck = inet_ntop(clnt_addr.ss_family, &clnt_addr, ipstr, sizeof(ipstr));
		if (ntopcheck == NULL) {
			fprintf(stderr, "inet_ntop error: %s\n", strerror(errno));
			return errno;
		}
		int getnameinfocheck = getnameinfo((struct sockaddr *)&clnt_addr, clnt_addr_len
			,hostname, NAME_MAX_LENGTH, servicename, NAME_MAX_LENGTH, 0);
		if (getnameinfocheck) {
			fprintf(stderr, "getnameinfo error: %s\n", gai_strerror(getnameinfocheck));
			return getnameinfocheck;
		}
		printf("ip: %s\nhost name: %s\nservice name: %s\nConnected\n", ipstr, hostname, servicename);
		
		int sendcheck;
		if ((sendcheck = file_send_athome(argv[1], clnt_fd))) {
			return sendcheck;
		}

		if (close(clnt_fd)) {
			fprintf(stderr, "close error: %s\n", strerror(errno));
			return errno;
		}
	}

	if (close(srv_fd)) {
		fprintf(stderr, "close error: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}
