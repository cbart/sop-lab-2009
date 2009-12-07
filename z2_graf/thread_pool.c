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
    if(pthread_condattr_init(&(new_pool->cond_attr)) != 0)
        return -1;
    if(pthread_cond_init(&new_pool->sleeping, &(new_pool->cond_attr)) != 0)
        return -1;
    new_pool->running_threads = 0;
    new_pool->max_running_threads = max_running_threads;
    new_pool->unused = NULL;
    for(i = 0; i < max_running_threads; i ++)
        new_pool->unused = stack_push(new_pool->unused, i);
    new_pool->sleeping_quantity = 0;
    return 0;
}

int thread_pool_wakeup_waiting(thread_pool *pool)
{
    assert(pool != NULL);
    if(pool->sleeping_quantity > 0) {
        pthread_cond_signal(&pool->sleeping);
        return 0;
    }
    else
        return 1;
}

void thread_pool_thread_sleep(thread_pool *pool)
{
    assert(pool != NULL);
    pool->sleeping_quantity ++;
}

void thread_pool_thread_awoken(thread_pool *pool)
{
    assert(pool != NULL);
    pool->sleeping_quantity --;
}

void stack_printf(thread_id_stack *stack)
{
    while(stack != NULL) {
        fprintf(stderr, "%d; ", (int) stack->id);
        stack = stack->next;
    }
}

pthread_t * thread_pool_get_free(thread_pool *pool)
{
    pthread_t *free_thread_place;
    assert(pool != NULL);
    fprintf(stderr, "Thread pool get free.\n");
    if(!stack_empty(pool->unused)) {
        //fprintf(stderr, "Thread pool get free, stack not empty.\n");
        free_thread_place = pool->threads + stack_top(pool->unused);
        //fprintf(stderr, "Thread pool get free, stack: ");
        //stack_printf(pool->unused);
        pool->unused = stack_pop(pool->unused);
        pool->running_threads ++;
        //fprintf(stderr, "\n");
        //stack_printf(pool->unused);
        //fprintf(stderr, "\n");
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
    assert(pool != NULL);
    if(pool->unused != NULL)
        stack_destroy(pool->unused);
    if(pthread_cond_destroy(&pool->sleeping) != 0)
            return -1;
    if(pthread_condattr_destroy(&(pool->cond_attr)))
        return -1;
    free(pool->threads);
    return 0;
}

