/** Thread pool.                                           *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <sys/types.h>
#include <pthread.h>

#include "customtypes.h"

typedef long thread_id;

/** Stack of thread ids. */
typedef struct thread_id_stack
{
    thread_id id;
    struct thread_id_stack *next;
} thread_id_stack;

/** Indicates if given `stack` is empty.
 * Returns:
 *   `0` if `stack` contains at least one element.
 *   `1` if `stack` is empty. */
int stack_empty(thread_id_stack *stack);

/** Pushes given `id` onto given `stack`.
 * Returns:
 *   `NULL` if encounters an error,
 *   pointer to new stack containing `id` in other cases. */
thread_id_stack * stack_push(thread_id_stack *stack, thread_id id);

/** Reads the top value of given `stack`.
 * Returns:
 *   `-1` if given stack is empty,
 *   stack top thread_id in other case. */
thread_id stack_top(thread_id_stack *stack);

/** Pops element out of stack.
 * If stack was empty (e.g. `NULL`) then it does nothing. */
thread_id_stack * stack_pop(thread_id_stack *stack);

/** Destroys given `stack`. */
void stack_destroy(thread_id_stack *stack);

/** Pool of free threads. */
typedef struct thread_pool
{
    pthread_t *threads;
    bool *working;
    pthread_condattr_t cond_attr;
    pthread_cond_t sleeping;
    long running_threads;
    long max_running_threads;
    thread_id_stack *unused;
    long sleeping_quantity;
} thread_pool;

/** Creates new thread pool.
 * Returns:
 *   `-1` in case of an error.
 *   `0` in other cases. */
int thread_pool_create(thread_pool *new_pool, long max_running_threads);

/** Wakes up a thread if there is one sleeping.
 * Returns:
 *   `1` if there were no sleeping threads.
 *   `0` if woke a sleeping thread.
 *   `-1` in case of an error. */
int thread_pool_wakeup_waiting(thread_pool *pool);

/** Makes a thread pool know that thread is sleeping. */
void thread_pool_thread_sleep(thread_pool *pool);

/** Makes a thread pool know that thread has awoken. */
void thread_pool_thread_awoken(thread_pool *pool);

/** Gets place for the new thread in the pool or `NULL` if there is no space. */
pthread_t * thread_pool_get_free(thread_pool *pool);

/** Makes a thread return to pool of free threads. */
void thread_pool_return_thread(thread_pool *pool, pthread_t *joined_thread);

/** Destroys thread pool pointed by `pool`. */
int thread_pool_destroy(thread_pool *pool);

#endif  /* _THREAD_POOL_H_ */
