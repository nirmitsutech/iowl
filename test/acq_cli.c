#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "acq_proto.h"
int create_client(char *addr,unsigned short int port)
{
	struct sockaddr_in sockaddr;
	int sock = -1;
     if( addr== NULL ) return -1;
	memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	sock = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr.sin_family = PF_INET;
	sockaddr.sin_port = htons(port);
	sockaddr.sin_addr.s_addr = inet_addr((const char *)addr);
	if (connect
		(sock, (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in)) < 0)
		return -1;
     else
             return sock;
}
