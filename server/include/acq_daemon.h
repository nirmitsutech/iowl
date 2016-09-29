#ifndef ACQ_DAEMON_H
#define ACQ_DAEMON_H
#include "acqd.h"
void handle_signals();
int setupdaemon();
#ifndef WIN32
void dosigterm(int sig);
#else
int daemon(int nochdir,int noclose);
#endif
#endif /*ACQ_DAEMON_H */
