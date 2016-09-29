#ifndef ACQ_TIMER_H
#define ACQ_TIMER_H
#ifdef WIN32
#include <sys/time.h>
enum __itimer_which
  {
    /* Timers run in real time.  */
    ITIMER_REAL = 0,
#define ITIMER_REAL ITIMER_REAL
    /* Timers run only when the process is executing.  */
    ITIMER_VIRTUAL = 1,
#define ITIMER_VIRTUAL ITIMER_VIRTUAL
    /* Timers run when the process is executing and when
       the system is executing on behalf of the process.  */
    ITIMER_PROF = 2
#define ITIMER_PROF ITIMER_PROF
  };
typedef enum __itimer_which __itimer_which_t;
struct itimerval
{
int isec;
struct timeval it_value;
struct timeval it_interval;
};
extern int gettimeoftheday(struct timeval *tvp,void *tzp);
extern int setitimer (__itimer_which_t __which,
                     __const struct itimerval *__restrict __new,
                      struct itimerval *__restrict __old);
#endif
#endif
