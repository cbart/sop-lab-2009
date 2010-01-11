/** Author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>
 *  Username: cb277617@students.mimuw.edu.pl
 *  */

#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <assert.h>
#include "sched_stack.h"
#include "err.h"

stack_t *mysched_stack_alloc(size_t bytes)
{
    void *allocated;
    stack_t *stack;
    if((allocated = malloc(bytes * BYTE_SIZE)) == NULL)
        return NULL;
    if((stack = (stack_t *) malloc(sizeof(stack_t))) == NULL) {
       free(allocated);
       return NULL;
    }
    stack->ss_sp = allocated;
    stack->ss_flags = SS_ONSTACK;
    stack->ss_size = bytes;
    return stack;
}

void mysched_stack_free(stack_t *stack_ptr)
{
    assert(stack_ptr != NULL);
    if(stack_ptr->ss_sp != NULL)
        free(stack_ptr->ss_sp);
    free(stack_ptr);
}
