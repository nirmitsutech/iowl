/****************************************************************************
 *
 * acqd - DATAACQUISION Managing Daemon
 * 
 * Copyright (C) Ganesh Gudigara <ganesh.gudigara@symbol.com>
 *
 *  =======
 *  HISTORY
 *  =======
 *
 * 0.1:  12/26/2005  Started the implementation this daemon based on lirc daemon
 *                   lirc architecture with lircd modified.
 *
 */




/*************************** INCLUDE CONFIG **********************************/

#include <acq_config.h>

/*****************************************************************************/


/************************************* ALL DEFINES ***************************/
#if !defined( _GNU_SOURCE ) && !defined( WIN32 )
#define _GNU_SOURCE
#endif
#if !defined( _BSD_SOURCE ) && !defined( WIN32 )
#define _BSD_SOURCE
#endif



/*****************************************************************************/


/*********************** ALL STANDARD INCLUDES *******************************/

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
#include "acq_log.h"
/*****************************************************************************/


/************************************ DEFINE MACRO FUNCIONS *****************/
/* cut'n'paste from fileutils-3.16: */

#define isodigit(c) ((c) >= '0' && (c) <= '7')

#ifndef timersub
#define timersub(a, b, result)                                            \
  do {                                                                    \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec;                         \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec;                      \
    if ((result)->tv_usec < 0) {                                          \
      --(result)->tv_sec;                                                 \
      (result)->tv_usec += 1000000;                                       \
    }                                                                     \
  } while (0)
#endif

/****************************************************************************/


/******************************** ACQD HEADERS ******************************/

#include "acqd.h"
#include "acq_device.h"
#include "acq_hw.h"

/****************************************************************************/



/*************************** ALL global and static initializations************/
static int enable = 1;
static sint8 *configfile = ACQCFGFILE;
static sint8 *pidfile = PIDFILE;
static sint8 *acqdfile = ACQD;


acqmanager_t acqm;
/******************************************************************************/


/***************************** All INLINE functions ***************************/



/******************************************************************************/




/******************************************************************************/


/************************** All static functions ****************************/

static int parse_args(int argc,char **argv,acqmanager_t *acq_m)
{


        return 0;
}

/* 
 * Return a positive integer containing the value of the 
 * ASCII octal number S.  If S is not an octal number, return -1.  
*/

static int oatoi(s)
	char *s;
{
	register int i;

	if (*s == 0)
		return -1;
	for (i = 0; isodigit(*s); ++s)
		i = i * 8 + *s - '0';
	if (*s)
		return -1;
	return i;
}


/******************************************************************************/



/*********************************** Global functions *************************/




void start_server(acqmanager_t * acqmanage)
{
	int fd=-1;
     int ret;
      

               ret=create_pidfile(pidfile);
               if( ret ) 
                    goto   failed0;
              fd=create_unixsocketandbind();
	         acqmanage->sockfd = fd;
              if(fd == -1) 
                      goto failed1;

               
	if (acqmanage->listen_tcpip) {
             acqmanage->sockinet=create_inetsocketandbind();
             enable=1;
             if(acqmanage->sockinet == -1) goto failed2;
             }
     
     #if  defined(USE_SYSLOG) && !defined(WIN32)
     #ifdef DAEMONIZE
	if (acqmanage->daemonized) {
		openlog(PROGNAME, LOG_CONS | LOG_PID | LOG_PERROR,
				DATAACQUISION_SYSLOG);
	} else {
		openlog(PROGNAME, LOG_CONS | LOG_PID, DATAACQUISION_SYSLOG);
	}
     #else
	openlog(PROGNAME, LOG_CONS | LOG_PID | LOG_PERROR, DATAACQUISION_SYSLOG);
     #endif
     #else
	openlogfile(PROGNAME, LOG_CONS | LOG_PID | LOG_PERROR, DATAACQUISION_SYSLOG);
     #endif
     
	return;

failed2:
	if (acqmanage->listen_tcpip) {
		close(acqmanage->sockinet);
	}
failed1:
	close(acqmanage->sockfd);
failed0:
     close_pidfile();
	exit(EXIT_FAILURE);
}


#ifdef DAEMONIZE
void daemonize(acqmanager_t * acqmanage)
{
	if (daemon(0, 0) == -1) {
		logprintf(LOG_ERR, "daemon() failed");
		logperror(LOG_ERR, NULL);
	}
     setupdaemon(acqmanage);
}
#endif							/* DAEMONIZE */




void loop(acqmanager_t * acqmanage)
{
	char *message;
	int len, i;

	//logprintf(LOG_NOTICE,"acqd(%s) ready",DATAACQUISION_DRIVER);
	while (1) {
		(void) waitfordata(acqmanage, 0);
		//if(!hw.rec_func) continue;
		//message=hw.rec_func(acqdevices);

		if (message != NULL) {
			len = strlen(message);

			for (i = 0; i < acqmanage->clin; i++) {
				LOGPRINTF(1, "writing to client %d", i);
				if (write_socket(acqmanage->clis[i], message, len) < len) {
					remove_client(acqmanage, acqmanage->clis[i]);
					i--;
				}
			}
		}
	}
}



/*****************************************************************************/


/************************************ main function **************************/

int main(int argc, char **argv)
{
	char *device = NULL;
     
     //Assign the manager
	acqmanager_t *acqmanage = &acqm;
     
#ifndef WIN32
	struct sigaction act;
     memset(&act,0,sizeof(struct sigaction));
	acqmanage->permission =
		S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
#endif
     
     //parse all arguments and take decisions
     parse_args(argc,argv,acqmanage);
     
     //Try to parse the device lists from config and probe the devices and 
     //Try to update/create config and update all structrures.
	init_hwlist(&device,configfile,acqmanage);
     
	if (device == NULL) {
             LOGERR("Not able to Initialize any device!!!!!");
             exit(EXIT_FAILURE);
     }
     
     #ifndef WIN32
	     signal(SIGPIPE, SIG_IGN);
     #endif
	start_server(acqmanage);
     
     //initialize signals
#ifndef WIN32
     init_signals(&act);
#endif
#ifdef DAEMONIZE
	/* ready to accept connections */
	if ( !acqmanage->daemonized )
		daemonize(acqmanage);
#endif

     //Loop For Ever
	loop(acqmanage);

     
     
	// never reached 
	return (EXIT_SUCCESS);
}

/********************************************* End ***************************/
