#ifndef FUNCS_H_
#define FUNCS_H_

int create_bind_stream_sock(int *srv_fd);
int file_send_athome(char* filename, int clntsock);
int request_prosses(char* request, char* file_name);
int print_clnt_info(struct sockaddr_storage* clnt_addr);

#endif