#include "defs.h"

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

#include "funcs.h"

int main(void)
{
	int srv_fd;
	int create_srv_ckeck;
	if ((create_srv_ckeck = sock_init(&srv_fd))) {
		return create_srv_ckeck;
	}

	int clnt_fd;
	struct sockaddr_storage clnt_addr;
	socklen_t clnt_addr_len = sizeof(clnt_addr);
	uint8_t recvbuffer[HTTP_REQUEST_SIZE];
	char file_name[SEND_FILE_NAME_SIZE];
	
	while (1) {		
		clnt_fd = accept(srv_fd, (struct sockaddr *)&clnt_addr, &clnt_addr_len);
		if (clnt_fd == -1) {
			fprintf(stderr, "accept error: %s\n", strerror(errno));
			if (close(clnt_fd)) {
				fprintf(stderr, "close error: %s\n", strerror(errno));
				return errno;
			}
			continue;
		}
		printf("\n--------------- client accepted -----------------\n");
		
		if (recv_athome(clnt_fd, recvbuffer, sizeof(recvbuffer))) {
			if (close(clnt_fd)) {
				fprintf(stderr, "close error: %s\n", strerror(errno));
				return errno;
			}
			continue;			
		}
		printf("\n%s", (char *)recvbuffer);

		if (print_clnt_info(&clnt_addr)) {
			if (close(clnt_fd)) {
				fprintf(stderr, "close error: %s\n", strerror(errno));
				return errno;
			}
			continue;
		}

		if (request_prosses((char*)recvbuffer, file_name)) {
			if (close(clnt_fd)) {
				fprintf(stderr, "close error: %s\n", strerror(errno));
				return errno;
			}
			continue;
		}
		printf("out file name: %s\n", file_name);

		if (file_send_athome(file_name, clnt_fd)) {
			if (close(clnt_fd)) {
				fprintf(stderr, "close error: %s\n", strerror(errno));
				return errno;
			}
			continue;
		}

		if (close(clnt_fd)) {
			fprintf(stderr, "close error: %s\n", strerror(errno));
			return errno;
		}
		printf("\n--------------- client served -----------------\n");
	}

	if (close(srv_fd)) {
		fprintf(stderr, "close error: %s\n", strerror(errno));
		return errno;
	}
	return 0;
}
