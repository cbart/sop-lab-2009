/** Author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>
 *  Username: cb277617@students.mimuw.edu.pl
 *  */

#include <sys/types.h>
#include <stdlib.h>
#include "sched_stack.h"
#include "err.h"

void *mysched_stack_alloc(size_t bytes)
{
     void *allocated;
     if((allocated = malloc(bytes * BYTE_SIZE)) == NULL)
         syserr("In mysched_stack_alloc: while allocating memory via
                 `malloc`.");
     return allocated;
}

void mysched_stack_free(void *stack_ptr)
{
    free(stack_ptr);
}
