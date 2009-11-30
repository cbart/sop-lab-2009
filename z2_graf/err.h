#ifndef _ERR_H_
#define _ERR_H_

#define WRITE_ERR "Error in `write` function!\n"
#define DUP_ERR "Error in `dup` function!\n"
#define CLS_ERR "Error in `close` function!\n"
#define PIPE_ERR "Error in `pipe` function!\n"
#define EXEC_ERR "Error in `exec` function!\n"

/* puts information about error in system function to stderr; */
extern void syserr(const char *fmt, ...);

/* puts information about error in application to stderr; */
extern void fatal(const char *fmt, ...);

#endif
