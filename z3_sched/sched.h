/** Author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>
 *  Username: cb277617@students.mimuw.edu.pl
 *  */

#ifndef _SCHED_H_
#define _SCHED_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

    /** Size of stack for one user-space thread (bytes). */
    #define MYSCHED_STACK_SIZE 1024*1024*1

    /** Threads id type. */
    typedef int mysched_thread_t;

    /** Library initiating procedure. */
    void mysched_init();

    /** Creates thread with given `name`; thread will launch `starter`. */
    mysched_thread_t mysched_create_thread(void (*starter)(), const char *name);

    /** Returns current thread id (when called after `myshed_go`. */
    mysched_thread_t mysched_self();

    /** Returns name of thread of given identificator. Caller is owner of
     *  returned memory. */
    char *mysched_get_name(mysched_thread_t thread);

    /** Asynchronous write. */
    ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset);

    /** Asynchronous read. */
    ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset);

    /** Launches created threads. */
    void mysched_go();

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif /* _SCHED_H_ */
