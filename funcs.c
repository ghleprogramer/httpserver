#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr_in6 sockaddr_in6;

#include "defs.h"

int create_bind_stream_sock(int *srv_fd)
{
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

	struct addrinfo *ptr = srv_info;
	int reuseaddr_opt = 1;
	while (ptr != NULL)
	{
		if ((*srv_fd = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol)) == -1) {
			fprintf(stderr, "socket error: %s\n", strerror(errno));
		} else if ((setsockopt(*srv_fd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr_opt, sizeof(reuseaddr_opt)))) {
			fprintf(stderr, "setsockopt error: %s\n", strerror(errno));
		} else if (bind(*srv_fd, ptr->ai_addr, ptr->ai_addrlen)) {
			fprintf(stderr, "bind error: %s\n", strerror(errno));
		} else {
			printf("bind done\n");
			break;
		}
		ptr = ptr->ai_next;
	}
	if (ptr == NULL) {
		printf("bind failed\n");
		return errno;
	}
	
	freeaddrinfo(srv_info);
	return 0;
}

int file_send_athome(char* filename, int clntsock)
{
	int msg_fd = open(filename, O_RDONLY);
	if (msg_fd == -1) {
		fprintf(stderr, "fstat error: %s\n", strerror(errno));
		return errno;
	}
	struct stat msg_stats;
	if (fstat(msg_fd, &msg_stats) == -1) {
		fprintf(stderr, "fstat error: %s\n", strerror(errno));
		return errno;
	}
	if (close(msg_fd)) {
		fprintf(stderr, "close error: %s\n", strerror(errno));
		return errno;
	}
	
	printf("to be sent file size: %lli\n", msg_stats.st_size);

	FILE *msg = fopen(filename, "rb");
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
			bytes_sent += send(clntsock, buff, bytes_read, 0);
			if (bytes_sent == -1) {
				fprintf(stderr, "send error: %s\n", strerror(errno));
				return errno;
			}
		} while (bytes_sent < bytes_read);
	}

	printf("file sent :)\n\n");
	if (fclose(msg)) {
		fprintf(stderr, "close error: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

int request_prosses(char* request, char* file_name)
{
	if (request == NULL) {
		fprintf(stderr, "request is NULL\n");
		return 1;
	}
	if (strcmp(strtok(request, " "), "GET")) {
		fprintf(stderr, "\n!!not a GET request!!\n");
		return 1;
	}

	char* input = strtok(NULL, " ");
	printf("\nget file: %s\n", input);

	if (strcmp(input, "/") == 0) {
		strcpy(file_name, "index.html");
		return 0;
	}

	while (*input == '/') {
		input += 1;
	}
	
	strncpy(file_name, input, SEND_FILE_NAME_SIZE);
	return 0;
}

char ipstr[INET6_ADDRSTRLEN];
char hostname[NAME_MAX_LENGTH];
char servicename[NAME_MAX_LENGTH];
int print_clnt_info(struct sockaddr_storage* clnt_addr)
{
	const char* ntopcheck = inet_ntop(clnt_addr->ss_family, clnt_addr, ipstr, sizeof(ipstr));
	if (ntopcheck == NULL) {
		fprintf(stderr, "inet_ntop error: %s\n", strerror(errno));
		return errno;
	}
	int getnameinfocheck = getnameinfo((struct sockaddr *)clnt_addr, sizeof(*clnt_addr)
		,hostname, NAME_MAX_LENGTH, servicename, NAME_MAX_LENGTH, 0);
	if (getnameinfocheck) {
		fprintf(stderr, "getnameinfo error: %s\n", gai_strerror(getnameinfocheck));
		return getnameinfocheck;
	}
	printf("ip: %s\nhost name: %s\nservice name: %s\nConnected\n", ipstr, hostname, servicename);
	return 0;
}

/* the rest are older test/learning funcs */

/*

int create_sock_bind_send(char *filetosend)
{
	struct addrinfo hints, *srv_info;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_protocol = PF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	
	int getaddrcheck = getaddrinfo(NULL, FUNCSSERVICE, &hints, &srv_info);
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

	// nothing is done with this rn
	uint32_t recvbuffer[FUNCSHTTP_RECV_SIZE];
	memset(recvbuffer, 0, sizeof(recvbuffer));
	if (recv(clnt_fd, recvbuffer, sizeof(recvbuffer), 0) == -1) {
		fprintf(stderr, "send error: %s\n", strerror(errno));
		return errno;
	}
	printf("%s\n", (char *)recvbuffer);

	// if (send(clnt_fd, HTTP_OK, sizeof(HTTP_OK), 0) == -1) {
	// 	fprintf(stderr, "send error: %s\n", strerror(errno));
	// 	return errno;
	// }

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
	
	FILE *msg = fopen(filetosend, "rb");
	if (msg == NULL) {
		fprintf(stderr, "fopen error: %s\n", strerror(errno));
		return errno;
	}
	uint8_t msgbuff;
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
		} while (bytes_sent < (int)sizeof(msgbuff));
	}

	close(clnt_fd);
	close(srv_fd);
	fclose(msg);
	
	return 0;
}

int printipaddrs(char* domain_name)
{
	struct addrinfo hints, *serverinfo, *ptr;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	int status = getaddrinfo(domain_name, "http", &hints, &serverinfo);
	if (status != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		return 1;
	}

	printf("ip addrs for %s are:\n", domain_name);

	char ipstr[INET6_ADDRSTRLEN];
	ptr = serverinfo;
	while (ptr != NULL)
	{
		void *addr;
		char *ipver;

		if (ptr->ai_family == AF_INET) {
			sockaddr_in *ipv4addr = (sockaddr_in *)ptr->ai_addr;
			addr = &(ipv4addr->sin_addr);
			ipver = "ipv4";
		} else {
			sockaddr_in6 *ipv6addr = (sockaddr_in6 *)ptr->ai_addr;
			addr = &(ipv6addr->sin6_addr);
			ipver = "ipv6";
		}

		inet_ntop(ptr->ai_family, addr, ipstr, sizeof(ipstr));
		printf("protocol: %s addr: %s\n", ipver, ipstr);

		ptr = ptr->ai_next;
	}
	
	freeaddrinfo(serverinfo);
	return 0;
}

*/