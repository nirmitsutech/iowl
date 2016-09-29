#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "acq_proto.h"
#include "acq_thread.h"
#define INETPORT 999
#define MAXCLI   100

void *(*readfunc)(void *);
main(int argc, char **argv)
{
        int lsize;
          static struct linger  linger = {0, 0};
        int clindex;
       char buffer[3];
	int clis = 0;
	int clilen;
	struct sockaddr_in sockaddr, cliaddr;
	int sock = -1;
     client_t clifd[MAXCLI];
	int fd = -1;
     readfunc=&readfromclient;
     
	memset(&sockaddr, 0, sizeof(struct sockaddr_in));
	sock = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr.sin_family = PF_INET;
	sockaddr.sin_port = htons(INETPORT);
	sockaddr.sin_addr.s_addr = inet_addr("192.168.0.1");
	if (bind(sock, (struct sockaddr *) &sockaddr, sizeof(struct sockaddr_in))
		< 0)
		return -1;
	listen(sock, 3);
                lsize  = sizeof(struct linger);
	setsockopt(sock, SOL_SOCKET, SO_LINGER, (void *)&linger, lsize);
             
	clilen = sizeof(struct sockaddr_in);
     clis=0;
	for (;;) {
		fd = -1;
		if (clis < 100)
			fd = accept(sock, (struct sockaddr *) &cliaddr, &clilen);
		if (fd >= 0)
          {
                  clifd[clis].fd=fd;
                  clifd[clis].buffer=NULL;
                  clifd[clis].buffersize=100;
                  fprintf(stderr,"fd=%d\n",clifd[clis].fd);
          spawn_newclient(0,readfunc,(void *)&clifd[clis]);
		clis++;
          }
          
          

	}
}
