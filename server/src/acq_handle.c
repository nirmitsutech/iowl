#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <errno.h>
#include <limits.h>
#include <fcntl.h>
#include <sys/file.h>


#include <signal.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <sys/time.h>
#ifndef WIN32
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#else
#include "acq_log.h"
#include <winsock2.h>
#endif
#include <sys/types.h>

#include "acq_daemon.h"
#include "acq_config.h"
#include "acqd.h"
#include "acq_handle.h"

INLINEINT write_socket(int fd, char *buf, int len)
{
	int done, todo = len;

	while (todo) {
		done = write(fd, buf, todo);
		if (done <= 0)
			return (done);
		buf += done;
		todo -= done;
	}
	return (len);
}

INLINEINT write_socket_len(int fd, char *buf)
{
	int len;

	len = strlen(buf);
	if (write_socket(fd, buf, len) < len)
		return (0);
	return (1);
}

INLINEINT read_timeout(int fd, char *buf, int len, int timeout)
{
	fd_set fds;
	struct timeval tv;
	int ret, n;

	FD_ZERO(&fds);
	FD_SET(fd, &fds);
	tv.tv_sec = timeout;
	tv.tv_usec = 0;

	/* CAVEAT: (from libc documentation)
	   Any signal will cause `select' to return immediately.  So if your
	   program uses signals, you can't rely on `select' to keep waiting
	   for the full time specified.  If you want to be sure of waiting
	   for a particular amount of time, you must check for `EINTR' and
	   repeat the `select' with a newly calculated timeout based on the
	   current time.  See the example below.

	   Obviously the timeout is not recalculated in the example because
	   this is done automatically on Linux systems...
	 */

	do {
		ret = select(fd + 1, &fds, NULL, NULL, &tv);
	} while (ret == -1 && errno == EINTR);
          if (ret == -1) {
                            logprintf(LOG_ERR, "select() failed");
                                      logperror(LOG_ERR, NULL);
                                                return (-1);
                                                     } else if (ret == 0)
                            return (0);         /* timeout */
          n = read(fd, buf, len);
               if (n == -1) {
                                 logprintf(LOG_ERR, "read() failed");
                                           logperror(LOG_ERR, NULL);
                                                     return (-1);
                                                          }
                    return (n);
}


void nolinger(int sock)
{



}

int init_signals(struct sigaction *act)
 {


         return 0;
 }



void remove_client(acqmanager_t * acqmanage, int fd)
{
	int index;

	for (index = 0; index < acqmanage->clin; index++) {
		if (acqmanage->clis[index] == fd) {
			shutdown(acqmanage->clis[index], 2);
			close(acqmanage->clis[index]);
			logprintf(LOG_INFO, "removed client");

			acqmanage->clin--;
			if (acqmanage->clin == 0) {
				//hw.deinit_func();
			}
			for (; index < acqmanage->clin; index++) {
				acqmanage->clis[index] = acqmanage->clis[index + 1];
			}
			return;
		}
	}
	LOGPRINTF(1, "internal error in remove_client: no such fd");
}

void add_client(acqmanager_t * acqmanage, int sock)
{
	int fd;
	int clilen;
	struct sockaddr client_addr;
	int flags;
#ifndef WIN32 
	clilen = sizeof(client_addr);
	fd = accept(sock, (struct sockaddr *) &client_addr, &clilen);
	if (fd == -1) {
		logprintf(LOG_ERR, "accept() failed for new client");
		logperror(LOG_ERR, NULL);
		dosigterm(SIGTERM);
	};

	if (fd >= FD_SETSIZE) {
		logprintf(LOG_ERR, "connection rejected");
		shutdown(fd, 2);
		close(fd);
		return;
	}
	nolinger(fd);
	flags = fcntl(fd, F_GETFL, 0);
	if (flags != -1) {
		fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	}
	if (client_addr.sa_family == AF_UNIX) {
		acqmanage->cli_type[acqmanage->clin] = CT_LOCAL;
		logprintf(LOG_NOTICE, "accepted new client on ");
	} else if (client_addr.sa_family == AF_INET) {
		acqmanage->cli_type[acqmanage->clin] = CT_REMOTE;
		logprintf(LOG_NOTICE, "accepted new client from %s",
				  inet_ntoa(((struct sockaddr_in *) &client_addr)->sin_addr));
	} else {
		acqmanage->cli_type[acqmanage->clin] = 0;	/* what? */
	}
	acqmanage->clis[acqmanage->clin++] = fd;
	if (acqmanage->clin == 1) {
		//if(hw.init_func)
		//{
		//if(!hw.init_func())
		//{
		shutdown(acqmanage->clis[0], 2);
		close(acqmanage->clis[0]);
		acqmanage->clin = 0;
		dosigterm(SIGTERM);
		//}
		//}
}
#endif
}

int add_peer_connection(acqmanager_t * acqmanage, char *server)
{
	char *sep;
	struct servent *service;

	if (acqmanage->peern < MAX_PEERS) {
		acqmanage->peers[acqmanage->peern] =
			malloc(sizeof(struct peer_connection));
		if (acqmanage->peers[acqmanage->peern] != NULL) {
			gettimeofday(&acqmanage->peers[acqmanage->peern]->reconnect,
						 NULL);
			acqmanage->peers[acqmanage->peern]->connection_failure = 0;
			sep = strchr(server, ':');
			if (sep != NULL) {
				*sep = 0;
				sep++;
				acqmanage->peers[acqmanage->peern]->host = strdup(server);
				service = getservbyname(sep, "tcp");
				if (service) {
					acqmanage->peers[acqmanage->peern]->port =
						ntohs(service->s_port);
				} else {
					long p;
					char *endptr;

					p = strtol(sep, &endptr, 10);
					if (!*sep || *endptr || p < 1 || p > USHRT_MAX) {
						LOGERR(
								"bad acqmanage->port number \"%s\"\n",
								sep);
						return (0);
					}

					acqmanage->peers[acqmanage->peern]->port =
						(unsigned short int) p;
				}
			} else {
				acqmanage->peers[acqmanage->peern]->host = strdup(server);
				acqmanage->peers[acqmanage->peern]->port = ACQ_INET_PORT;
			}
			if (acqmanage->peers[acqmanage->peern]->host == NULL) {
				LOGERR("out of memory\n");
			}
		} else {
			LOGERR(" out of memory\n");
			return (0);
		}
		acqmanage->peers[acqmanage->peern]->socket = -1;
		acqmanage->peern++;
		return (1);
	} else {
		 LOGERR("too many client connections\n");
	}
	return (0);
}

void connect_to_peers(acqmanager_t * acqmanage)
{
	int i;
	struct hostent *host;
	struct sockaddr_in addr;
	struct timeval now;
	int enable = 1;

	gettimeofday(&now, NULL);
	for (i = 0; i < acqmanage->peern; i++) {
		if (acqmanage->peers[i]->socket != -1)
			continue;
		/* some timercmp() definitions don't work with <= */
		if (timercmp(&acqmanage->peers[i]->reconnect, &now, <)) {
			acqmanage->peers[i]->socket = socket(AF_INET, SOCK_STREAM, 0);
			host = gethostbyname(acqmanage->peers[i]->host);
			if (host == NULL) {
				logprintf(LOG_ERR, "name lookup failure "
						  "connecting to %s", acqmanage->peers[i]->host);
				acqmanage->peers[i]->connection_failure++;
				gettimeofday(&acqmanage->peers[i]->reconnect, NULL);
				acqmanage->peers[i]->reconnect.tv_sec +=
					5 * acqmanage->peers[i]->connection_failure;
				close(acqmanage->peers[i]->socket);
				acqmanage->peers[i]->socket = -1;
				continue;
			}

#ifndef WIN32 
			(void) setsockopt(acqmanage->peers[i]->socket, SOL_SOCKET,
							  SO_KEEPALIVE, &enable, sizeof(enable));
#endif
			addr.sin_family = host->h_addrtype;;
			addr.sin_addr = *((struct in_addr *) host->h_addr);
			addr.sin_port = htons(acqmanage->peers[i]->port);
			if (connect
				(acqmanage->peers[i]->socket, (struct sockaddr *) &addr,
				 sizeof(addr)) == -1) {
				logprintf(LOG_ERR, "failure connecting to %s",
						  acqmanage->peers[i]->host);
				logperror(LOG_ERR, NULL);
				acqmanage->peers[i]->connection_failure++;
				gettimeofday(&acqmanage->peers[i]->reconnect, NULL);
				acqmanage->peers[i]->reconnect.tv_sec +=
					5 * acqmanage->peers[i]->connection_failure;
				close(acqmanage->peers[i]->socket);
				acqmanage->peers[i]->socket = -1;
				continue;
			}
			logprintf(LOG_NOTICE, "connected to %s",
					  acqmanage->peers[i]->host);
			acqmanage->peers[i]->connection_failure = 0;
		}
	}
}

sint32 get_peer_message(acqmanager_t * acqmanage,
						struct peer_connection *peer)
{
	int length;
	char buffer[PACKET_SIZE + 1];
	char *end;
	int i;

	length = read_timeout(peer->socket, buffer, PACKET_SIZE, 0);
	if (length) {
		buffer[length] = 0;
		end = strchr(buffer, '\n');
		if (end == NULL) {
			logprintf(LOG_ERR, "bad send packet: \"%s\"", buffer);
			/* remove clients that behave badly */
			return (0);
		}
		end++;			/* include the \n */
		end[0] = 0;
		LOGPRINTF(1, "received peer message: \"%s\"", buffer);
		for (i = 0; i < acqmanage->clin; i++) {
			/* don't relay messages to acqdev clients */
			if (acqmanage->cli_type[i] == CT_REMOTE)
				continue;
			LOGPRINTF(1, "writing to client %d", i);
			if (write_socket(acqmanage->clis[i], buffer, length) < length) {
				remove_client(acqmanage, acqmanage->clis[i]);
				i--;
			}
		}
	}

	if (length == 0) {	/* EOF: connection closed by client */
		return (0);
	}
	return (1);
}

int waitfordata(acqmanager_t * acqmanage, long maxusec)
{
	fd_set fds;
	int maxfd, i, ret, reconnect;
	struct timeval tv, start, now;

	while (1) {
		do {
			/* handle signals */
                  handle_signals();
			FD_ZERO(&fds);
			FD_SET(acqmanage->sockfd, &fds);

			maxfd = acqmanage->sockfd;
			if (acqmanage->listen_tcpip) {
				FD_SET(acqmanage->sockinet, &fds);
                    maxfd = (maxfd>acqmanage->sockinet)?maxfd:acqmanage->sockinet;
			}
               
			if (acqmanage->clin > 0) {
			     ;
               }

			for (i = 0; i < acqmanage->clin; i++) {
			     ;
               }
			if (reconnect) {
				connect_to_peers(&acqm);
			}
		}
		while (ret == -1 && errno == EINTR);

		for (i = 0; i < acqmanage->clin; i++) {
			if (FD_ISSET(acqmanage->clis[i], &fds)) {
				FD_CLR(acqmanage->clis[i], &fds);
				if (get_command(acqmanage->clis[i]) == 0) {
					remove_client(acqmanage, acqmanage->clis[i]);
					i--;
				}
			}
		}
		for (i = 0; i < acqmanage->peern; i++) {
			if (acqmanage->peers[i]->socket != -1 &&
				FD_ISSET(acqmanage->peers[i]->socket, &fds)) {
				if (get_peer_message(acqmanage, acqmanage->peers[i]) == 0) {
					shutdown(acqmanage->peers[i]->socket, 2);
					close(acqmanage->peers[i]->socket);
					acqmanage->peers[i]->socket = -1;
					acqmanage->peers[i]->connection_failure = 1;
					gettimeofday(&acqmanage->peers[i]->reconnect, NULL);
					acqmanage->peers[i]->reconnect.tv_sec += 5;
				}
			}
		}

		if (FD_ISSET(acqmanage->sockfd, &fds)) {
			LOGPRINTF(1, "registering local client");
			add_client(acqmanage, acqmanage->sockfd);
		}
		if (acqmanage->listen_tcpip && FD_ISSET(acqmanage->sockinet, &fds)) {
			LOGPRINTF(1, "registering inet client");
			add_client(acqmanage, acqmanage->sockinet);
		}
		if (acqmanage->clin >
			0 /*&& hw.rec_mode!=0 && FD_ISSET(hw.fd,&fds) */ ) {
			/* we will read later */
			return (1);
		}
	}
}

int create_unixsocketandbind()
{
return 0;
}
int create_inetsocketandbind()
{
return 0;
}
int get_command(int fd)
{
return 0;
}
