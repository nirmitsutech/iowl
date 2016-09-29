#include <stdio.h>
#include <signal.h>
#include <sys/time.h>
#include "acqd.h"
#include "acq_daemon.h"
#include "acq_timer.h"

FILE *pidf = NULL;
static sig_atomic_t term = 0, hup = 0, alrm = 0;
static int termsig;
int trmsig()
{
        return termsig;
}
sig_atomic_t signal_term()
{
        return term;
}
sig_atomic_t signal_alrm()
{
        return alrm;
}
sig_atomic_t signal_hup()
{
        return hup;
}


#ifndef WIN32

void sigterm(int sig)
{
	/* all signals are blocked now */
	if (term)
		return;
	term = 1;
	termsig = sig;
}

void sigalrm(int sig)
{
	alrm = 1;
}


void dosighup(int sig)
{


}



void dosigalrm(int sig)
{
	int repeat_fd;
	char repeat_message[12];
	struct itimerval repeat_timer;

	if (acqm.clin == 0) {
		;
	}
	if (1) {
		repeat_timer.it_value.tv_sec = 0;
		repeat_timer.it_value.tv_usec = 0;	
		repeat_timer.it_interval.tv_sec = 0;
		repeat_timer.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL, &repeat_timer, NULL);
		return;
	}
	if (repeat_fd != -1) {
		send_success(repeat_fd, &repeat_message[0]);
		//free(repeat_message);
		//repeat_message=NULL;
		repeat_fd = -1;
	}
	if (acqm.clin == 0 /*&& repeat_acqdev==NULL && hw.deinit_func */ ) {
		//hw.deinit_func();
	}


}

void dosigterm(int sig)
{

}

#else
int daemon(int nochdir, int noclose)
{

return 0;
}
#endif
void handle_signals()
{

}
int setupdaemon(acqmanager_t *acq_m)
{
return 0;
}
        
int create_pidfile()
{
return 0;
}

int close_pidfile()
{
return 0;
}
