#ifndef _ERR_H_
#define _ERR_H_

/* Puts information about error in system function to stderr. */
extern void syserr(const char *fmt, ...);

/* Puts information about error in application to stderr. */
extern void fatal(const char *fmt, ...);

#endif
