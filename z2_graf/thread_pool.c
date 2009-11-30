#include <sys/types.h>
#include <pthread.h>
#include <assert.h>

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
    while(!stack_empty(stack))
        stack = stack_pop(stack);
}

int thread_pool_create(thread_pool *new_pool, long max_running_threads)
{
    if((new_pool->threads = (pthread_t *) calloc(sizeof(pthread_t),
                    (size_t) max_running_threads)) == NULL)
        return -1;
    new_pool->running_threads = 0;
    new_pool->max_running_threads = max_running_threads;
    new_pool->stack = NULL;
    return 0;
}

pthread_t * thread_pool_get_free(thread_pool *pool)
{
    pthread_t *free_thread_place;
    assert(pool != NULL);
    if(!stack_empty(pool->stack)) {
        free_thread_place = pool->threads + stack_top(pool->stack);
        stack_pop(pool->stack);
        return free_thread_place;
    }
    else if(pool->running_threads < pool->max_running_threads) {
        free_thread_place = pool->threads + pool->running_threads;
        pool->running_threads
    }
}

void thread_pool_destroy(thread_pool *pool)
{
    assert(pool != NULL);
    free(pool->threads);
    if(thread_pool->stack != NULL)
        stack_destroy(thread_pool->stack);
}

