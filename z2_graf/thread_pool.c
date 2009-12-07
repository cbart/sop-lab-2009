/** Thread pool.                                           *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdio.h>

#include "thread_pool.h"

int stack_empty(thread_id_stack *stack)
{
    return (stack == NULL);
}

thread_id_stack * stack_push(thread_id_stack *stack, thread_id id)
{
    thread_id_stack *new_stack;
    if((new_stack = malloc(sizeof(thread_id_stack))) == NULL)
        return NULL;
    new_stack->id = id;
    new_stack->next = stack;
    return new_stack;
}

thread_id stack_top(thread_id_stack *stack)
{
    if(stack == NULL)
        return -1;
    return stack->id;
}

thread_id_stack * stack_pop(thread_id_stack *stack)
{
    thread_id_stack *new_stack;
    if(stack == NULL)
        return stack;
    new_stack = stack->next;
    stack->next = NULL;
    free(stack);
    return new_stack;
}

void stack_destroy(thread_id_stack *stack)
{
    thread_id_stack *tmp = stack;
    while(!stack_empty(tmp)) {
        tmp = stack_pop(tmp);
    }
}

int thread_pool_create(thread_pool *new_pool, long max_running_threads)
{
    thread_id i;
    if((new_pool->threads = (pthread_t *) calloc(sizeof(pthread_t),
                    (size_t) max_running_threads)) == NULL)
        return -1;
    if((new_pool->sleeping = (pthread_cond_t *) calloc(sizeof(pthread_cond_t),
                    (size_t) max_running_threads)) == NULL)
        return -1;
    if(pthread_condattr_init(&(new_pool->cond_attr)) != 0)
        return -1;
    for(i = 0; i < max_running_threads; i ++)
        if(pthread_cond_init(new_pool->sleeping + i,
                    &(new_pool->cond_attr)) != 0)
            return -1;
    new_pool->running_threads = 0;
    new_pool->max_running_threads = max_running_threads;
    new_pool->unused = NULL;
    for(i = 0; i < max_running_threads; i ++)
        new_pool->unused = stack_push(new_pool->unused, i);
    new_pool->waiting = NULL;
    return 0;
}

pthread_t * thread_pool_get_waiting(thread_pool *pool)
{
    pthread_t *waiting_thread;
    assert(pool != NULL);
    if(!stack_empty(pool->waiting)) {
        waiting_thread = pool->threads + stack_top(pool->waiting);
        stack_pop(pool->waiting);
        return waiting_thread;
    }
    return NULL;
}

pthread_t * thread_pool_get_free(thread_pool *pool)
{
    pthread_t *free_thread_place;
    assert(pool != NULL);
    if(!stack_empty(pool->unused)) {
        free_thread_place = pool->threads + stack_top(pool->unused);
        stack_pop(pool->unused);
        pool->running_threads ++;
        return free_thread_place;
    }
    return NULL;
}

void thread_pool_return_thread(thread_pool *pool, pthread_t *joined_thread)
{
    assert(pool != NULL);
    assert(pool->threads <= joined_thread && joined_thread < pool->threads +
            pool->max_running_threads);
    pool->unused = stack_push(pool->unused, joined_thread - pool->threads);
}

int thread_pool_destroy(thread_pool *pool)
{
    size_t i;
    assert(pool != NULL);
    if(pool->unused != NULL)
        stack_destroy(pool->unused);
    if(pool->waiting != NULL)
        stack_destroy(pool->waiting);
    for(i = 0; i < pool->max_running_threads; i ++) {
        if(pthread_cond_destroy(pool->sleeping + i) != 0)
            return -1;
    }
    if(pthread_condattr_destroy(&(pool->cond_attr)))
        return -1;
    free(pool->sleeping);
    free(pool->threads);
    return 0;
}

