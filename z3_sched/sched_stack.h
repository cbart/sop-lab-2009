/** Author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>
 *  Username: cb277617@students.mimuw.edu.pl
 *  */

#ifndef _SCHED_STACK_H_
#define _SCHED_STACK_H_

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif  /* __cplusplus */

    typedef unsigned char byte;

    #define BYTE_SIZE sizeof(byte)

    /** Allocates `bytes` of memory for a new stack. */
    stack_t *mysched_stack_alloc(size_t bytes);

    /** Frees memory allocated by `alloc_stack`. */
    void mysched_stack_free(stack_t *stack_ptr);

#ifdef __cplusplus
}
#endif  /* __cplusplus */

#endif  /* _SCHED_STACK_H_ */
