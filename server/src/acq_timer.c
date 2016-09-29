#ifdef WIN32
#include "acq_timer.h"
#include <sys/timeb.h>
int gettimeofday(struct timeval *tvp, void *tzp)
{
	struct timeb timebuffer;

	ftime(&timebuffer);
	if (tvp) {
		tvp->tv_sec = timebuffer.time;
		tvp->tv_usec = timebuffer.millitm * 1000L;
	}
	return (0);
}

int setitimer (__itimer_which_t __which,
                     __const struct itimerval *__restrict __new,
                      struct itimerval *__restrict __old)
{
return 0;
}
#endif

