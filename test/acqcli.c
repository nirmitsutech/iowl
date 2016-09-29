#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "acq_proto.h"
#define INETPORT 999
main(int argc, char **argv)
{
	char buffer[3];
	struct sockaddr_in sockaddr;
	int sock = -1;
	memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	sock = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr.sin_family = PF_INET;
	sockaddr.sin_port = htons(INETPORT);
	sockaddr.sin_addr.s_addr = inet_addr("192.168.0.1");
	if (connect
		(sock, (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in)) < 0)
		return -1;

	for (;;) {

		create_basecmd(CMD_DISPLAYMODE, &buffer[0], 3);
		sendbuffer(sock, &buffer[0], 3, 0);
		printf("Send Buffers\n");
	}
}
