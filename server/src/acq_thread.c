#include "acq_thread.h"
#include "acq_proto.h"
#include <stdio.h>
#include <stdlib.h>
static pthread_cond_t *gReadCompleteConditionPtr = NULL;
pthread_cond_t readCompleteCondition;
static pthread_mutex_t *gReadMutexPtr = NULL;
pthread_mutex_t readMutex;



int spawn_newclient(int alloc, void *(*threadfunc) (void *),
					 client_t *reference)
{
	int status = 0;
	pthread_t thr;
	int thread_created = 0;
	if (alloc) {
          if(reference != NULL)
		reference->buffer = malloc(reference->buffersize);
		if ( reference != NULL  && !reference->buffer) {
			fprintf(stderr, "ERROR: Couldn't allocate internal buffer\n");
			status = -1;
			return status;
		}
          if(reference != NULL)
                  reference->alloc=1;
	}

	fprintf(stderr, "INFO: Create a New Thread...\n");

	if (pthread_cond_init(&readCompleteCondition, NULL) == 0) {
		gReadCompleteConditionPtr = &readCompleteCondition;

		if (pthread_mutex_init(&readMutex, NULL) == 0) {
			gReadMutexPtr = &readMutex;

			if (pthread_create(&thr, NULL, threadfunc,(void *)reference) > 0)
				fprintf(stderr, "WARNING: Couldn't create read Thread\n");
			else
				thread_created = 1;
		}
	}

	if (thread_created) {
		pthread_join(thr, NULL);	/* wait for the child thread to return */
	}
     fprintf(stderr,"Created The new thread\n");
	return status;
}


void readfromclient(void *ref)
{
        client_t *pcli=(client_t *)ref;
        if(pcli == NULL) return ;
        fprintf(stderr,"GETTING THE THREAD %d\n",pcli->fd);
	for (;;) {
         fprintf(stderr,"Check for the data\n");     
         recvbuffer(pcli->fd,&pcli->buffer[0],3,0);
         printf("Changing Everything %02X %02X %02X\n",pcli->buffer[0],pcli->buffer[1],pcli->buffer[2]);
         pcli->buffer[0]=0;
             
	}
     
}
