#include <stdio.h>
#include <stdarg.h>
#include "sys/time.h"
#include "acq_log.h"
#include "acq_timer.h"
#include "acq_config.h"
#include "acqd.h"
#include <errno.h>

#ifndef USE_SYSLOG
#define HOSTNAME_LEN 128

static sint8 *logfile = LOGFILE;
sint8 hostname[HOSTNAME_LEN + 1];
static FILE *lf=NULL;
static sint8 *progname = PROGNAME;

void openlogfile(char *prog,int prio,int facilitate)
{

}
#ifndef WIN32
void logprintf(int prio, char *format_str, ...)
{
	time_t current;
	char *currents;
	va_list ap;

	current = time(&current);
	currents = ctime(&current);

	if (lf)
		fprintf(lf, "%15.15s %s %s: ", currents + 4, hostname, progname);
	if (!acqm.daemonized)
		fprintf(stderr, "%s: ", progname);
	va_start(ap, format_str);
	if (lf) {
		if (prio == LOG_WARNING)
			fprintf(lf, "WARNING: ");
		vfprintf(lf, format_str, ap);
		fputc('\n', lf);
		fflush(lf);
	}
	if (!acqm.daemonized) {
		if (prio == LOG_WARNING)
			fprintf(stderr, "WARNING: ");
		vfprintf(stderr, format_str, ap);
		fputc('\n', stderr);
		fflush(stderr);
	}
	va_end(ap);
}

void logperror(int prio, const char *s)
{
	if (s != NULL) {
		logprintf(prio, "%s: %s", s, strerror(errno));
	} else {
		logprintf(prio, "%s", strerror(errno));
	}
}
#endif
#endif
