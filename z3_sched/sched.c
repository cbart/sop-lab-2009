/** Author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>
 *  Username: cb277617@students.mimuw.edu.pl
 *  */

#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#include "err.h"
#include "sched.h"
#include "sched_stack.h"
#include "sched_queue.h"

/* TYPES: */

typedef enum mysched_thread_state {
    READY,
    RUNNING,
    WAITING
} mysched_thread_state_t;

typedef struct mysched_thread_info {
    /** Identificator. */
    mysched_thread_t id;
    /** Thread's stack. */
    stack_t *stack;
    /** Thread name. */
    char *name;
    /** Thread state. */
    mysched_thread_state_t state;
    sigjmp_buf jump_point;
    /** Thread closure. */
    void (*fun)();
} mysched_thread_info_t;

typedef struct mysched_pool_t
{
    queue_entry_t *working;
    queue_entry_t *waiting;
    int next_thread_id;

} mysched_pool_t;

/* GLOBALS: */

#define SAVE_SIGS 1
#define DONT_SAVE_SIGS 0

#define THREAD_START 1
#define THREAD_CHANGE 2
#define THREAD_CONTINUE 3

static mysched_pool_t pool;

/* SUBROUTINES: */

void mysched_sig_handler_create(int signum)
{
    mysched_thread_info_t *self_info;
    int jmp_ret;
    queue_get(&pool.working, (void *) self_info);
    switch(jmp_ret = sigsetjmp(self_info->jump_point, SAVE_SIGS)) {
        case 0:  /* Set jump return. */
            break;
        case THREAD_START:  /* Long jump. */
            (*(self_info->fun))();
            break;
        default:
            /* Error. */
            assert(0);
            break;
    }
    return;
}

void mysched_init()
{
    queue_init(&pool.working);
    queue_init(&pool.waiting);
    pool.next_thread_id = 0;
    return;
}

mysched_thread_t mysched_create_thread(void (*starter)(), const char *name)
{
    stack_t *thread_stack = NULL;
    stack_t *def_stack = NULL;
    mysched_thread_info_t *self_info = NULL;
    struct sigaction action;
    struct sigaction old_act;
    int signum = SIGUSR1;
    size_t name_len = strlen(name);

    assert(starter != NULL);
    assert(name != NULL);

    if((thread_stack = mysched_stack_alloc(MYSCHED_STACK_SIZE)) == NULL)
        return -1;
    if(sigaltstack(thread_stack, def_stack) != 0)
        return -2;
    if((self_info = (mysched_thread_info_t *)
                malloc(sizeof(mysched_thread_info_t))) == NULL)
        return -3;

    self_info->id = pool.next_thread_id ++;
    self_info->stack = thread_stack;
    if((self_info->name =
                (char *) malloc((name_len + 1) * sizeof(char))) == NULL)
        return -4;
    strncpy(self_info->name, name, name_len);
    self_info->name[name_len] = '\0';
    self_info->state = READY;
    self_info->fun = starter;

    if(queue_add_before(&pool.working, (void *) self_info) != 0) {
        free(self_info);
        return -5;
    }

    action.sa_handler = mysched_sig_handler_create;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_ONSTACK;

    if(sigaction(SIGUSR1, &action, &old_act) != 0)
        return -6;
    raise(SIGUSR1);

    return 0;
}

mysched_thread_t mysched_self()
{
    return 0;
}

char *mysched_get_name(mysched_thread_t thread)
{
    char* result = (char*) malloc(sizeof(char));
    result[0] = 'c';
    return result;
}

ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset)
{
    return 0;
}

ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset)
{
    return 0;
}

void mysched_go()
{
    mysched_thread_info_t *clean_info = NULL;

    /* Cleaning. */
    while(!queue_is_empty(pool.working)) {
        if(queue_get(&pool.working, clean_info) != 0)
            fatal("Error: getting elements from empty queue!");
        queue_shift_forward(&pool.working);
        queue_rm_before(&pool.working);
        free(clean_info->name);
        mysched_stack_free(clean_info->stack);
        free(clean_info);
    }
    return;
}
