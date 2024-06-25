#define _POSIX_C_SOURCE 200112L

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

typedef struct sockaddr_in sockaddr_in;
typedef struct sockaddr_in6 sockaddr_in6;

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