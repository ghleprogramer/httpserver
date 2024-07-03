#include "defs.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

struct addrinfo hints, *srv_info, *ptr;
int getaddrcheck;
int reuseaddr_opt;
int sock_init(int *srv_fd)
{
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_flags = AI_PASSIVE;
	hints.ai_socktype = SOCK_STREAM;
	
	if ((getaddrcheck = getaddrinfo(NULL, SERVICE, &hints, &srv_info))) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(getaddrcheck));
		return getaddrcheck;
	}

	ptr = srv_info;
	reuseaddr_opt = 1;
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

	if (listen(*srv_fd, LISTEN_QUEUE) == -1) {
		fprintf(stderr, "listen error: %s\n", strerror(errno));
		return errno;
	}

	return 0;
}

int msg_fd;
struct stat msg_stats;
FILE *msg;
uint8_t buff[SEND_BUFFER_SIZE];
long bytes_read;
long bytes_sent;
int file_send_athome(char* filename, int clntsock)
{
	msg_fd = open(filename, O_RDONLY);
	if (msg_fd == -1) {
		fprintf(stderr, "open error: %s\n", strerror(errno));
		return errno;
	}
	if (fstat(msg_fd, &msg_stats) == -1) {
		fprintf(stderr, "fstat error: %s\n", strerror(errno));
		return errno;
	}
	if (close(msg_fd)) {
		fprintf(stderr, "close error: %s\n", strerror(errno));
		return errno;
	}
	
	printf("to be sent file size: %lli\n", msg_stats.st_size);

	msg = fopen(filename, "rb");
	if (msg == NULL) {
		fprintf(stderr, "fopen error: %s\n", strerror(errno));
		return errno;
	}
	
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

	printf("file sent :)\n");
	if (fclose(msg)) {
		fprintf(stderr, "close error: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}

char* input;
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

	input = strtok(NULL, " ");
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
int getnameinfocheck;
int print_clnt_info(struct sockaddr_storage* clnt_addr)
{
	if (inet_ntop(clnt_addr->ss_family, clnt_addr, ipstr, sizeof(ipstr)) == NULL) {
		fprintf(stderr, "inet_ntop error: %s\n", strerror(errno));
		return errno;
	}
	if ((getnameinfocheck = getnameinfo((struct sockaddr *)clnt_addr, sizeof(*clnt_addr)
		,hostname, NAME_MAX_LENGTH, servicename, NAME_MAX_LENGTH, 0))) {
		fprintf(stderr, "getnameinfo error: %s\n", gai_strerror(getnameinfocheck));
		return getnameinfocheck;
	}
	printf("ip: %s\nhost name: %s\nservice name: %s\nConnected\n", ipstr, hostname, servicename);
	return 0;
}
