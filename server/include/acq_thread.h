#ifndef ACQ_THREAD_H
#define ACQ_THREAD_H
#include <pthread.h>
typedef struct client_s
{
int alloc;
int fd;
char *buffer;
int buffersize;
}client_t;
int spawn_newclient(int alloc, void *(*threadfunc) (void *),
					 client_t *reference);
void readfromclient(void *ref);
#endif
