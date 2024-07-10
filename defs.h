#ifndef DEFS_H_
#define DEFS_H_

#define _POSIX_C_SOURCE 200112L

#define LISTEN_QUEUE 10
#define SERVICE "3490"
#define NAME_MAX_LENGTH 50
#define HTTP_REQUEST_SIZE 512
#define SEND_BUFFER_SIZE 1000*10 // 10KB
#define SEND_FILE_NAME_SIZE 100
#define REUSE_SOCK_ADDR 1
#define STATIC_STR_LEN 7
#define HTTP_OK "HTTP/1.1 200 OK\r\n"
#define MAX_CONTENT_TYPE 100
#define CONTENT_HTML "Content-Type: text/html\r\n\r\n"
#define CONTENT_ICO "Content-Type: image/vnd.microsoft.icon\r\n\r\n"
#define CONTENT_MP4 "Content-Type: video/mp4\r\n\r\n"

#endif
