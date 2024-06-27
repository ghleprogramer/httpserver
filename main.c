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

	struct addrinfo hints, *srv_info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	
	int getaddrcheck = getaddrinfo(NULL, SERVICE, &hints, &srv_info);
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

	int reuseaddr_opt = 1;
	int sockoptcheck = setsockopt(srv_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt));
	if (sockoptcheck == -1) {
		fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
		return errno;
	}

	if (bind(srv_fd, srv_info->ai_addr, srv_info->ai_addrlen) == -1) {
		fprintf(stderr, "bind error: %s\n", strerror(errno));
		return errno;
	}
	freeaddrinfo(srv_info);

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
	
	int msg_fd = open(argv[1], O_RDONLY);
	if (msg_fd == -1) {
		fprintf(stderr, "fstat error: %s\n", strerror(errno));
		return errno;
	}
	struct stat msg_stats;
	if (fstat(msg_fd, &msg_stats) == -1) {
		fprintf(stderr, "fstat error: %s\n", strerror(errno));
		return errno;
	}
	close(msg_fd);

	printf("sent file size:%i\n\n", (int)msg_stats.st_size);

	FILE *msg = fopen(argv[1], "rb");
	if (msg == NULL) {
		fprintf(stderr, "fopen error: %s\n", strerror(errno));
		return errno;
	}
	
	uint8_t buff[SEND_BUFFER_SIZE];
	long bytes_read;
	long bytes_sent;
	while ((bytes_read = fread(buff, sizeof(buff[0]), sizeof(buff), msg))) {
		if (ferror(msg)) {
			fprintf(stderr, "fread error: %s\n", strerror(errno));
			return errno;
		}
		bytes_sent = 0;
		do
		{
			bytes_sent += send(clnt_fd, buff, bytes_read, 0);
			if (bytes_sent == -1) {
				fprintf(stderr, "send error: %s\n", strerror(errno));
				return errno;
			}
		} while (bytes_sent < bytes_read);
	}
	fclose(msg);
	
	close(clnt_fd);
	close(srv_fd);
	
	return 0;
}
