#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#ifdef HAVE_WIN32_THREADS
#include <windows.h>
#ifndef HAVE_COMPILER_TLS
#include <process.h>
#endif
#endif

#ifdef HAVE_BEOS_THREADS
#include <OS.h>
#include <TLS.h>
#endif

#if defined(SOLARIS)
#include <note.h>
#endif

struct _acqMutex {
    pthread_mutex_t lock;
#if defined HAVE_WIN32_THREADS
    HANDLE mutex;
#elif defined HAVE_BEOS_THREADS
	sem_id sem;
	thread_id tid;
#else
    int empty;
#endif
};

/*
 * acqRMutex are reentrant mutual exception locks
 */
struct _acqRMutex {
    pthread_mutex_t lock;
    unsigned int    held;
    unsigned int    waiters;
    pthread_t       tid;
    pthread_cond_t  cv;
#if defined HAVE_WIN32_THREADS
    CRITICAL_SECTION cs;
    unsigned int count;
#elif defined HAVE_BEOS_THREADS
	acqMutexPtr lock;
	thread_id tid;
	int32 count;
#else
    int empty;
#endif
};
/*
 * This module still has some internal static data.
 *   - acqCommLock a global lock
 *   - globalkey used for per-thread data
 */

static pthread_key_t	globalkey;
static pthread_t	mainthread;
static pthread_once_t once_control = PTHREAD_ONCE_INIT;
#if defined HAVE_WIN32_THREADS
#if defined(HAVE_COMPILER_TLS)
static __declspec(thread) acqGlobalState tlstate;
static __declspec(thread) int tlstate_inited = 0;
#else /* HAVE_COMPILER_TLS */
static DWORD globalkey = TLS_OUT_OF_INDEXES;
#endif /* HAVE_COMPILER_TLS */
static DWORD mainthread;
static int run_once_init = 1;
/* endif HAVE_WIN32_THREADS */
#elif defined HAVE_BEOS_THREADS
int32 globalkey = 0;
thread_id mainthread = 0;
int32 run_once_init = 0;
#endif

static acqRMutexPtr	acqCommLock = NULL;
static void acqOnceInit(void);

/**
 * acqNewMutex:
 *
 * acqNewMutex() is used to allocate a libacq2 token struct for use in
 * synchronizing access to data.
 *
 * Returns a new simple mutex pointer or NULL in case of error
 */
acqMutexPtr
acqNewMutex(void)
{
    acqMutexPtr tok;

    if ((tok = malloc(sizeof(acqMutex))) == NULL)
        return (NULL);
    pthread_mutex_init(&tok->lock, NULL);
#if defined HAVE_WIN32_THREADS
    tok->mutex = CreateMutex(NULL, FALSE, NULL);
#elif defined HAVE_BEOS_THREADS
	if ((tok->sem = create_sem(1, "acqMutex")) < B_OK) {
		free(tok);
		return NULL;
	}
	tok->tid = -1;
#endif
    return (tok);
}

/**
 * acqFreeMutex:
 * @tok:  the simple mutex
 *
 * acqFreeMutex() is used to reclaim resources associated with a libacq2 token
 * struct.
 */
void
acqFreeMutex(acqMutexPtr tok)
{
    if (tok == NULL) return;

    pthread_mutex_destroy(&tok->lock);
#if defined HAVE_WIN32_THREADS
    CloseHandle(tok->mutex);
#elif defined HAVE_BEOS_THREADS
	delete_sem(tok->sem);
#endif
    free(tok);
}

/**
 * acqMutexLock:
 * @tok:  the simple mutex
 *
 * acqMutexLock() is used to lock a libacq2 token.
 */
void
acqMutexLock(acqMutexPtr tok)
{
    if (tok == NULL)
        return;
    pthread_mutex_lock(&tok->lock);
#if defined HAVE_WIN32_THREADS
    WaitForSingleObject(tok->mutex, INFINITE);
#elif defined HAVE_BEOS_THREADS
	if (acquire_sem(tok->sem) != B_NO_ERROR) {
#ifdef DEBUG_THREADS
		acqGenericError(acqGenericErrorContext, "acqMutexLock():BeOS:Couldn't aquire semaphore\n");
		exit();
#endif
	}
	tok->tid = find_thread(NULL);
#endif

}

/**
 * acqMutexUnlock:
 * @tok:  the simple mutex
 *
 * acqMutexUnlock() is used to unlock a libacq2 token.
 */
void
acqMutexUnlock(acqMutexPtr tok)
{
    if (tok == NULL)
        return;
    pthread_mutex_unlock(&tok->lock);
#if defined HAVE_WIN32_THREADS
    ReleaseMutex(tok->mutex);
#elif defined HAVE_BEOS_THREADS
	if (tok->tid == find_thread(NULL)) {
		tok->tid = -1;
		release_sem(tok->sem);
	}
#endif
}

/**
 * acqNewRMutex:
 *
 * acqRNewMutex() is used to allocate a reentrant mutex for use in
 * synchronizing access to data. token_r is a re-entrant lock and thus useful
 * for synchronizing access to data structures that may be manipulated in a
 * recursive fashion.
 *
 * Returns the new reentrant mutex pointer or NULL in case of error
 */
acqRMutexPtr
acqNewRMutex(void)
{
    acqRMutexPtr tok;

    if ((tok = malloc(sizeof(acqRMutex))) == NULL)
        return (NULL);
#ifdef HAVE_PTHREAD_H
    pthread_mutex_init(&tok->lock, NULL);
    tok->held = 0;
    tok->waiters = 0;
    pthread_cond_init(&tok->cv, NULL);
#elif defined HAVE_WIN32_THREADS
    InitializeCriticalSection(&tok->cs);
    tok->count = 0;
#elif defined HAVE_BEOS_THREADS
	if ((tok->lock = acqNewMutex()) == NULL) {
		free(tok);
		return NULL;
	}
	tok->count = 0;
#endif
    return (tok);
}

/**
 * acqFreeRMutex:
 * @tok:  the reentrant mutex
 *
 * acqRFreeMutex() is used to reclaim resources associated with a
 * reentrant mutex.
 */
void
acqFreeRMutex(acqRMutexPtr tok ATTRIBUTE_UNUSED)
{
#ifdef HAVE_PTHREAD_H
    pthread_mutex_destroy(&tok->lock);
#elif defined HAVE_WIN32_THREADS
    DeleteCriticalSection(&tok->cs);
#elif defined HAVE_BEOS_THREADS
	acqFreeMutex(tok->lock);
#endif
    free(tok);
}

/**
 * acqRMutexLock:
 * @tok:  the reentrant mutex
 *
 * acqRMutexLock() is used to lock a libacq2 token_r.
 */
void
acqRMutexLock(acqRMutexPtr tok)
{
    if (tok == NULL)
        return;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock(&tok->lock);
    if (tok->held) {
        if (pthread_equal(tok->tid, pthread_self())) {
            tok->held++;
            pthread_mutex_unlock(&tok->lock);
            return;
        } else {
            tok->waiters++;
            while (tok->held)
                pthread_cond_wait(&tok->cv, &tok->lock);
            tok->waiters--;
        }
    }
    tok->tid = pthread_self();
    tok->held = 1;
    pthread_mutex_unlock(&tok->lock);
#elif defined HAVE_WIN32_THREADS
    EnterCriticalSection(&tok->cs);
    ++tok->count;
#elif defined HAVE_BEOS_THREADS
	if (tok->lock->tid == find_thread(NULL)) {
		tok->count++;
		return;
	} else {
		acqMutexLock(tok->lock);
		tok->count = 1;
	}
#endif
}

/**
 * acqRMutexUnlock:
 * @tok:  the reentrant mutex
 *
 * acqRMutexUnlock() is used to unlock a libacq2 token_r.
 */
void
acqRMutexUnlock(acqRMutexPtr tok ATTRIBUTE_UNUSED)
{
    if (tok == NULL)
        return;
#ifdef HAVE_PTHREAD_H
    pthread_mutex_lock(&tok->lock);
    tok->held--;
    if (tok->held == 0) {
        if (tok->waiters)
            pthread_cond_signal(&tok->cv);
        tok->tid = 0;
    }
    pthread_mutex_unlock(&tok->lock);
#elif defined HAVE_WIN32_THREADS
    if (!--tok->count) 
	LeaveCriticalSection(&tok->cs);
#elif defined HAVE_BEOS_THREADS
	if (tok->lock->tid == find_thread(NULL)) {
		tok->count--;
		if (tok->count == 0) {
			acqMutexUnlock(tok->lock);
		}
		return;
	}
#endif
}

/************************************************************************
 *									*
 *			Per thread global state handling		*
 *									*
 ************************************************************************/

#ifdef LIBXML_THREAD_ENABLED
#ifdef acqLastError
#undef acqLastError
#endif
/**
 * acqFreeGlobalState:
 * @state:  a thread global state
 *
 * acqFreeGlobalState() is called when a thread terminates with a non-NULL
 * global state. It is is used here to reclaim memory resources.
 */
static void
acqFreeGlobalState(void *state)
{
    acqGlobalState *gs = (acqGlobalState *) state;

    /* free any memory allocated in the thread's acqLastError */
    acqResetError(&(gs->acqLastError));
    free(state);
}

/**
 * acqNewGlobalState:
 *
 * acqNewGlobalState() allocates a global state. This structure is used to
 * hold all data for use by a thread when supporting backwards compatibility
 * of libacq2 to pre-thread-safe behaviour.
 *
 * Returns the newly allocated acqGlobalStatePtr or NULL in case of error
 */
static acqGlobalStatePtr
acqNewGlobalState(void)
{
    acqGlobalState *gs;
    
    gs = malloc(sizeof(acqGlobalState));
    if (gs == NULL)
	return(NULL);

    memset(gs, 0, sizeof(acqGlobalState));
    acqInitializeGlobalState(gs);
    return (gs);
}
#endif /* LIBXML_THREAD_ENABLED */


#ifdef HAVE_WIN32_THREADS
#if !defined(HAVE_COMPILER_TLS)
#if defined(LIBXML_STATIC) && !defined(LIBXML_STATIC_FOR_DLL)
typedef struct _acqGlobalStateCleanupHelperParams
{
    HANDLE thread;
    void *memory;
} acqGlobalStateCleanupHelperParams;

static void acqGlobalStateCleanupHelper (void *p)
{
    acqGlobalStateCleanupHelperParams *params = (acqGlobalStateCleanupHelperParams *) p;
    WaitForSingleObject(params->thread, INFINITE);
    CloseHandle(params->thread);
    acqFreeGlobalState(params->memory);
    free(params);
    _endthread();
}
#else /* LIBXML_STATIC && !LIBXML_STATIC_FOR_DLL */

typedef struct _acqGlobalStateCleanupHelperParams
{
    void *memory;
    struct _acqGlobalStateCleanupHelperParams * prev;
    struct _acqGlobalStateCleanupHelperParams * next;
} acqGlobalStateCleanupHelperParams;

static acqGlobalStateCleanupHelperParams * cleanup_helpers_head = NULL;
static CRITICAL_SECTION cleanup_helpers_cs;

#endif /* LIBXMLSTATIC && !LIBXML_STATIC_FOR_DLL */
#endif /* HAVE_COMPILER_TLS */
#endif /* HAVE_WIN32_THREADS */

#if defined HAVE_BEOS_THREADS
/**
 * acqGlobalStateCleanup:
 * @data: unused parameter
 *
 * Used for Beos only
 */
void acqGlobalStateCleanup(void *data)
{
	void *globalval = tls_get(globalkey);
	if (globalval != NULL)
		acqFreeGlobalState(globalval);
}
#endif

/**
 * acqGetGlobalState:
 *
 * acqGetGlobalState() is called to retrieve the global state for a thread.
 *
 * Returns the thread global state or NULL in case of error
 */
acqGlobalStatePtr
acqGetGlobalState(void)
{
#ifdef HAVE_PTHREAD_H
    acqGlobalState *globalval;

    pthread_once(&once_control, acqOnceInit);

    if ((globalval = (acqGlobalState *)
		pthread_getspecific(globalkey)) == NULL) {
        acqGlobalState *tsd = acqNewGlobalState();

        pthread_setspecific(globalkey, tsd);
        return (tsd);
    }
    return (globalval);
#elif defined HAVE_WIN32_THREADS
#if defined(HAVE_COMPILER_TLS)
    if (!tlstate_inited) {
	tlstate_inited = 1;
	acqInitializeGlobalState(&tlstate);
    }
    return &tlstate;
#else /* HAVE_COMPILER_TLS */
    acqGlobalState *globalval;
    acqGlobalStateCleanupHelperParams * p;

    if (run_once_init) { 
	run_once_init = 0; 
	acqOnceInit(); 
    }
#if defined(LIBXML_STATIC) && !defined(LIBXML_STATIC_FOR_DLL)
    globalval = (acqGlobalState *)TlsGetValue(globalkey);
#else
    p = (acqGlobalStateCleanupHelperParams*)TlsGetValue(globalkey);
    globalval = (acqGlobalState *)(p ? p->memory : NULL);
#endif
    if (globalval == NULL) {
	acqGlobalState *tsd = acqNewGlobalState();
	p = (acqGlobalStateCleanupHelperParams *) malloc(sizeof(acqGlobalStateCleanupHelperParams));
	p->memory = tsd;
#if defined(LIBXML_STATIC) && !defined(LIBXML_STATIC_FOR_DLL)
	DuplicateHandle(GetCurrentProcess(), GetCurrentThread(), 
		GetCurrentProcess(), &p->thread, 0, TRUE, DUPLICATE_SAME_ACCESS);
	TlsSetValue(globalkey, tsd);
	_beginthread(acqGlobalStateCleanupHelper, 0, p);
#else
	EnterCriticalSection(&cleanup_helpers_cs);	
        if (cleanup_helpers_head != NULL) {
            cleanup_helpers_head->prev = p;
        }
	p->next = cleanup_helpers_head;
	p->prev = NULL;
	cleanup_helpers_head = p;
	TlsSetValue(globalkey, p);
	LeaveCriticalSection(&cleanup_helpers_cs);	
#endif

	return (tsd);
    }
    return (globalval);
#endif /* HAVE_COMPILER_TLS */
#elif defined HAVE_BEOS_THREADS
    acqGlobalState *globalval;

    acqOnceInit();

    if ((globalval = (acqGlobalState *)
		tls_get(globalkey)) == NULL) {
        acqGlobalState *tsd = acqNewGlobalState();

        tls_set(globalkey, tsd);
        on_exit_thread(acqGlobalStateCleanup, NULL);
        return (tsd);
    }
    return (globalval);
#else
    return(NULL);
#endif
}

/************************************************************************
 *									*
 *			Comm wide thread interfaces			*
 *									*
 ************************************************************************/

/**
 * acqGetThreadId:
 *
 * acqGetThreadId() find the current thread ID number
 *
 * Returns the current thread ID number
 */
int
acqGetThreadId(void)
{
#ifdef HAVE_PTHREAD_H
    return((int) pthread_self());
#elif defined HAVE_WIN32_THREADS
    return GetCurrentThreadId();
#elif defined HAVE_BEOS_THREADS
	return find_thread(NULL);
#else
    return((int) 0);
#endif
}

/**
 * acqIsMainThread:
 *
 * acqIsMainThread() check whether the current thread is the main thread.
 *
 * Returns 1 if the current thread is the main thread, 0 otherwise
 */
int
acqIsMainThread(void)
{
#ifdef HAVE_PTHREAD_H
    pthread_once(&once_control, acqOnceInit);
#elif defined HAVE_WIN32_THREADS
    if (run_once_init) { 
	run_once_init = 0; 
	acqOnceInit (); 
    }
#elif defined HAVE_BEOS_THREADS
	acqOnceInit();
#endif
        
#ifdef DEBUG_THREADS
    acqGenericError(acqGenericErrorContext, "acqIsMainThread()\n");
#endif
#ifdef HAVE_PTHREAD_H
    return(mainthread == pthread_self());
#elif defined HAVE_WIN32_THREADS
    return(mainthread == GetCurrentThreadId ());
#elif defined HAVE_BEOS_THREADS
	return(mainthread == find_thread(NULL));
#else
    return(1);
#endif
}

/**
 * acqLockComm:
 *
 * acqLockComm() is used to take out a re-entrant lock on the libacq2
 * library.
 */
void
acqLockComm(void)
{
#ifdef DEBUG_THREADS
    acqGenericError(acqGenericErrorContext, "acqLockComm()\n");
#endif
    acqRMutexLock(acqCommLock);
}

/**
 * acqUnlockComm:
 *
 * acqUnlockComm() is used to release a re-entrant lock on the libacq2
 * library.
 */
void
acqUnlockComm(void)
{
#ifdef DEBUG_THREADS
    acqGenericError(acqGenericErrorContext, "acqUnlockComm()\n");
#endif
    acqRMutexUnlock(acqCommLock);
}

/**
 * acqInitThreads:
 *
 * acqInitThreads() is used to to initialize all the thread related
 * data of the libacq2 library.
 */
void
acqInitThreads(void)
{
#ifdef DEBUG_THREADS
    acqGenericError(acqGenericErrorContext, "acqInitThreads()\n");
#endif
#if defined(HAVE_WIN32_THREADS) && !defined(HAVE_COMPILER_TLS) && (!defined(LIBXML_STATIC) || defined(LIBXML_STATIC_FOR_DLL))
    InitializeCriticalSection(&cleanup_helpers_cs);
#endif
}

/**
 * acqCleanupThreads:
 *
 * acqCleanupThreads() is used to to cleanup all the thread related
 * data of the libacq2 library once processing has ended.
 */
void
acqCleanupThreads(void)
{
#ifdef DEBUG_THREADS
    acqGenericError(acqGenericErrorContext, "acqCleanupThreads()\n");
#endif
#if defined(HAVE_WIN32_THREADS) && !defined(HAVE_COMPILER_TLS) && (!defined(LIBXML_STATIC) || defined(LIBXML_STATIC_FOR_DLL))
    if (globalkey != TLS_OUT_OF_INDEXES) {
	acqGlobalStateCleanupHelperParams * p;
	EnterCriticalSection(&cleanup_helpers_cs);
	p = cleanup_helpers_head;
	while (p != NULL) {
		acqGlobalStateCleanupHelperParams * temp = p;
		p = p->next;
		acqFreeGlobalState(temp->memory);
		free(temp);
	}
	cleanup_helpers_head = 0;
	LeaveCriticalSection(&cleanup_helpers_cs);
	TlsFree(globalkey);
	globalkey = TLS_OUT_OF_INDEXES;
    }
    DeleteCriticalSection(&cleanup_helpers_cs);
#endif
}

#ifdef LIBXML_THREAD_ENABLED
/**
 * acqOnceInit
 *
 * acqOnceInit() is used to initialize the value of mainthread for use
 * in other routines. This function should only be called using
 * pthread_once() in association with the once_control variable to ensure
 * that the function is only called once. See man pthread_once for more
 * details.
 */
static void
acqOnceInit(void) {
#ifdef HAVE_PTHREAD_H
    (void) pthread_key_create(&globalkey, acqFreeGlobalState);
    mainthread = pthread_self();
#endif

#if defined(HAVE_WIN32_THREADS)
#if !defined(HAVE_COMPILER_TLS)
    globalkey = TlsAlloc();
#endif
    mainthread = GetCurrentThreadId();
#endif

#ifdef HAVE_BEOS_THREADS
	if (atomic_add(&run_once_init, 1) == 0) {
		globalkey = tls_allocate();
		tls_set(globalkey, NULL);
		mainthread = find_thread(NULL);
	} else
		atomic_add(&run_once_init, -1);
#endif
}
#endif

/**
 * DllMain:
 * @hinstDLL: handle to DLL instance
 * @fdwReason: Reason code for entry
 * @lpvReserved: generic pointer (depends upon reason code)
 *
 * Entry point for Windows library. It is being used to free thread-specific
 * storage.
 *
 * Returns TRUE always
 */
#if defined(HAVE_WIN32_THREADS) && !defined(HAVE_COMPILER_TLS) && (!defined(LIBXML_STATIC) || defined(LIBXML_STATIC_FOR_DLL))
#if defined(LIBXML_STATIC_FOR_DLL)
BOOL WINAPI acqDllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
#else
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) 
#endif
{
    switch(fdwReason) {
    case DLL_THREAD_DETACH:
	if (globalkey != TLS_OUT_OF_INDEXES) {
	    acqGlobalState *globalval = NULL;
	    acqGlobalStateCleanupHelperParams * p =
		(acqGlobalStateCleanupHelperParams*)TlsGetValue(globalkey);
	    globalval = (acqGlobalState *)(p ? p->memory : NULL);
            if (globalval) {
                acqFreeGlobalState(globalval);
                TlsSetValue(globalkey,NULL);
            }
	    if (p)
	    {
		EnterCriticalSection(&cleanup_helpers_cs);
                if (p == cleanup_helpers_head)
		    cleanup_helpers_head = p->next;
                else
		    p->prev->next = p->next;
                if (p->next != NULL)
                    p->next->prev = p->prev;
		LeaveCriticalSection(&cleanup_helpers_cs);
		free(p);
	    }
	}
	break;
    }
    return TRUE;
}
#endif
