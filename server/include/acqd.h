/*
 *
 *
 *
 *
 *
 * 
 */

#ifndef _ACQD_H
#define _ACQD_H
#include "acq_daemon.h"
#include "acq_handle.h"

#if !defined(WIN32) 
#include <syslog.h>
#endif
#ifdef WIN32
#include "acq_timer.h"
#endif
#include <sys/time.h>
#include "acq_device.h"
#include "acq_config.h"
#define DEBUGLEVEL 1
#define PACKET_SIZE (256)
#define WHITE_SPACE " \t"
#define CT_LOCAL  1
#define CT_REMOTE 2
#define MAX_PEERS	100
#if !defined(WIN32)
#define MAX_CLIENTS (FD_SETSIZE-5-MAX_PEERS)
#else
#define MAX_CLIENTS (250-5-MAX_PEERS)
#endif




struct peer_connection {
	char *host;
	unsigned short port;
	struct timeval reconnect;
	int connection_failure;
	int socket;
};

typedef struct acqmanager {
	sint32 sockfd, sockinet;
	sint32 clis[MAX_CLIENTS];
	sint32 cli_type[MAX_CLIENTS];
	sint32 clin;
	sint32 listen_tcpip;
	uint16 port;
	struct peer_connection *peers[MAX_PEERS];
	sint32 peern;
	sint32 debug;
	sint32 daemonized;
     mode_t permission;
	char *lockfile;
     config_mgr_t *mgr;
} acqmanager_t;


void sigterm(int sig);
void dosigterm(int sig);
void sighup(int sig);
void dosighup(int sig);
int config(char *configfile,config_mgr_t *mgr);
void nolinger(int sock);
void remove_client(acqmanager_t *, int fd);
void add_client(acqmanager_t *, int);
sint32 add_peer_connection(acqmanager_t *, char *server);
void connect_to_peers(acqmanager_t *);
sint32 get_peer_message(acqmanager_t *, struct peer_connection *peer);
void start_server(acqmanager_t *);

#ifdef DEBUG
#define LOGPRINTF(level,fmt,args...)	\
  if(level<=DEBUGLEVEL) logprintf(LOG_DEBUG,fmt, ## args )
#define LOGPERROR(level,s) \
  if(level<=DEBUGLEVEL) logperror(LOG_DEBUG,s)
#else
#define LOGPRINTF(level,fmt,args...)	\
  do {} while(0)
#define LOGPERROR(level,s) \
  do {} while(0)
#endif

#ifdef USE_SYSLOG
#define logprintf syslog
#define logperror(prio,s) if((s)!=NULL) syslog(prio,"%s: %m\n",(char *) s); else syslog(prio,"%m\n")
#else
#ifndef WIN32
void logprintf(int prio, char *format_str, ...);
void logperror(int prio, const char *s);
#else
#define logprintf(prio,format_str,args...) fprintf(stderr,format_str,##args)
#define logperror(prio,format_str) fprintf(stderr,format_str)
#endif
#endif


void daemonize(acqmanager_t *);
void sigalrm(int sig);
void dosigalrm(int sig);
int parse_rc(int fd, char *message, char *arguments,
			 int n);
int send_success(int fd, char *message);
int send_error(int fd, char *message, char *format_str, ...);
int send_acqdev_list(int fd, char *message);
int send_acqdev(int fd, char *message);
int send_name(int fd, char *message, void *code);
int list(int fd, char *message, char *arguments);
int send_once(int fd, char *message, char *arguments);
int send_start(int fd, char *message, char *arguments);
int send_stop(int fd, char *message, char *arguments);
int send_core(int fd, char *message, char *arguments, int once);
int version(int fd, char *message, char *arguments);
int get_pid(int fd, char *message, char *arguments);
int get_command(int fd);
int waitfordata(acqmanager_t *, long maxusec);
void loop(acqmanager_t *);


struct protocol_directive {
	char *name;
	int (*function) (int fd, char *message, char *arguments);
};

extern acqmanager_t acqm;
#endif							/* _ACQD_H */
