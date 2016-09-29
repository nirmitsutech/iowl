#ifndef ACQ_H
#define ACQ_H
#define DEBUG 1
#if defined DEBUG & __KERNEL__
#define DPRINT(fmt,args...) printk(KERN_DEBUG,fmt,##args)
#else
#ifdef DEBUG
#define DPRINT(fmt,args...) fprintf(stderr,fmt,##args)
#else
#define DPRINT(fmt,args...)
#endif
#endif
#define LOGMSG(fmt,args...) fprintf(stderr,fmt,##args)
#define LOGERR(fmt,args...) fprintf(stderr,fmt,##args)

#ifdef WIN32
typedef inline int INLINEINT;
typedef inline void INLINEVOID;
#else
typedef int INLINEINT;
typedef int INLINEVOID;
#endif
#define max(a,b) ((a)>(b))?(a):(b)


/*********************  ALL TYPES RENAMED!!!! ****************************/
typedef char sint8;
typedef unsigned char uint8;
typedef short int sint16;
typedef unsigned short int uint16;
typedef int sint32;
typedef unsigned int uint32;
typedef long long sint64;
typedef unsigned long long uint64;

/***************************************************************************/

#endif /*ACQ_H */
