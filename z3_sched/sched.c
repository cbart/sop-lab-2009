/** Author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>
 *  Username: cb277617@students.mimuw.edu.pl
 *  */

#include <stdlib.h>
#include <setjmp.h>
#include <signal.h>
#include <sys/time.h>
#include <string.h>
#include <assert.h>
#include "err.h"
#include "sched.h"
#include "sched_stack.h"
#include "sched_queue.h"

/* TYPES: */

typedef enum mysched_thread_state {
    NEW,
    READY,
    AIO_WAITING
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
    int not_waiting_q;
    queue_entry_t *working;
    queue_entry_t *waiting;
    int next_thread_id;
    int rm_flag;

} mysched_pool_t;

/* GLOBALS: */

#define SAVE_SIGS 1
#define DONT_SAVE_SIGS 0

#define THREAD_START 1
#define THREAD_CHANGE 2
#define THREAD_CONTINUE 3

static mysched_pool_t pool;
static sigjmp_buf aio_wait_jump_point;
static sigjmp_buf go_jump_point;
static struct itimerval timerval;
static struct itimerval quick_timerval;

/* SUBROUTINES: */

void mysched_sig_handler_preemp(int signum)
{
    printf("::preemp.\n");
    mysched_thread_info_t *self_info = NULL;
    mysched_thread_info_t *jump_info = NULL;
    int jmp_ret = 0;
    if(queue_is_empty(pool.working)) {
        printf("**preemp: pool queue empty.\n");
        if(queue_is_empty(pool.waiting)) {
            printf("**preemp: go long jump.\n");
            siglongjmp(go_jump_point, 1);  /* All threads completed. */
        }
        else {
            jmp_ret = sigsetjmp(aio_wait_jump_point, SAVE_SIGS);
            if(jmp_ret == 0)
                return;
        }
    }
    if(jmp_ret == 0) {  /* Changing context. */
        if(queue_get(&pool.working, (void **) &self_info) != 0)
            fatal("(EE) Reading empty queue!\n");
        if((self_info->state != NEW) && (pool.rm_flag == 0)) {
            printf("**preemp: set jump, thread: %d.\n", self_info->id);
            jmp_ret = sigsetjmp(self_info->jump_point, SAVE_SIGS);
        }
        setitimer(ITIMER_PROF, &timerval, NULL);
        switch(jmp_ret) {
            case 0:  /* Return from sigsetjmp: switch context. */
                printf("**preemp: shifting queue forward.\n");
                if(pool.rm_flag == 0)
                    queue_shift_forward(&pool.working);
                else
                    pool.rm_flag = 0;
                printf("**preemp: getting queue element: ");
                if(queue_get(&pool.working, (void **) &jump_info) != 0)
                    fatal("(EE) Reading empty queue!\n");
                printf("thread id is %d.\n", jump_info->id);
                printf("**preemp: siglongjmp!!\n");
                if(jump_info->state == NEW)
                    siglongjmp(jump_info->jump_point, THREAD_START);
                else
                    siglongjmp(jump_info->jump_point, THREAD_CONTINUE);
            case THREAD_CONTINUE:  /* Longjmp: resume thread. */
                return;
        }
    }
    else {  /* Starting thread waiting on AIO. */
        
    }
    printf("--preemp\n");
}

void mysched_sig_handler_create(int signum)
{
    printf("::create\n");
    mysched_thread_info_t *self_info = NULL;
    mysched_thread_info_t *clean_info = NULL;
    int jmp_ret = 0;
    if(queue_get(&pool.working, (void **) &self_info) != 0)
        fatal("(EE) Reading empty queue!\n");
    jmp_ret = sigsetjmp(self_info->jump_point, SAVE_SIGS);
    printf("Setjmp %d, at thread %d.\n", jmp_ret, self_info->id);
    if(jmp_ret == THREAD_START) {  /* Long jump. */
        self_info->state = READY;
        printf("**create: fun start.\n");
        (*(self_info->fun))();
        printf("**create: fun end.\n");
        /* Clean thread info. */
        if(queue_get(&pool.working, (void **) &clean_info) != 0)
            fatal("(EE) Reading empty queue!\n");
        assert(self_info == clean_info);
        printf("**create: shifting queue.\n");
        queue_shift_forward(&pool.working);
        pool.rm_flag = 1;
        printf("**create: rm thread %d from queue.\n", clean_info->id);
        queue_rm_before(&pool.working);
        //free(clean_info->name);
        //mysched_stack_free(clean_info->stack);
        //free(clean_info);
        pool.not_waiting_q--;
        printf("**create: raising sigprof, not_waiting: %d.\n",
                pool.not_waiting_q);
        raise(SIGPROF);
    }
    else
        assert(jmp_ret == 0);  /* Has to be setjmp return. */
    printf("--create.\n");
   // if(queue_is_empty(pool.working) && queue_is_empty(pool.waiting))
   //     siglongjmp();
}

void mysched_init()
{
    queue_init(&pool.working);
    queue_init(&pool.waiting);
    timerval.it_value.tv_sec = 0;
    timerval.it_value.tv_usec = 1000;
    quick_timerval.it_value.tv_sec = 0;
    quick_timerval.it_value.tv_usec = 0;
    quick_timerval.it_interval.tv_sec = 0;
    quick_timerval.it_interval.tv_usec = 0;
    pool.next_thread_id = 0;
    pool.rm_flag = 0;
    return;
}

mysched_thread_t mysched_create_thread(void (*starter)(), const char *name)
{
    stack_t *thread_stack = NULL;  /* Sig. handler's custom stack. */
    stack_t *def_stack = NULL;  /* Here we store sig. handler's def stack. */
    mysched_thread_info_t *self_info = NULL;  /* Thread information. */
    struct sigaction action;
    struct sigaction action_preemp;
    struct sigaction old_act;
    const int signum = SIGUSR1;
    const size_t name_len = strlen(name);

    assert(starter != NULL);
    assert(name != NULL);

    if((thread_stack = mysched_stack_alloc(MYSCHED_STACK_SIZE)) == NULL)
        return -1;
    if(sigaltstack(thread_stack, def_stack) != 0)
        return -2;
    if((self_info = (mysched_thread_info_t *)
                malloc(sizeof(mysched_thread_info_t))) == NULL)
        return -3;

    self_info->id = pool.next_thread_id++;
    printf("Setting id: %d\n", pool.next_thread_id - 1);
    self_info->stack = thread_stack;
    if((self_info->name =
                (char *) malloc((name_len + 1) * sizeof(char))) == NULL)
        return -4;
    strncpy(self_info->name, name, name_len);
    self_info->name[name_len] = '\0';
    self_info->state = NEW;
    self_info->fun = starter;

    if(queue_add_before(&pool.working, (void *) self_info) != 0) {
        //free(self_info);
        return -5;
    }
    pool.not_waiting_q++;
    queue_shift_backward(&pool.working);

    action.sa_handler = mysched_sig_handler_create;
    sigemptyset(&action.sa_mask);
    action.sa_flags = SA_ONSTACK;

    if(sigaction(signum, &action, &old_act) != 0)
        return -6;
    if(raise(signum) != 0)
        return -7;

    action_preemp.sa_handler = mysched_sig_handler_preemp;
    action_preemp.sa_flags = 0;
    sigemptyset(&action_preemp.sa_mask);
    if(sigaction(SIGPROF, &action_preemp, &old_act) != 0)
        return -8;

    return 0;
}

mysched_thread_t mysched_self()
{
    mysched_thread_info_t *self_info = NULL;
    if(queue_get(&pool.working, (void **) &self_info) != 0)
        return -1;  /* Queue empty. */
    return self_info->id;
}

char *mysched_get_name(mysched_thread_t thread)
{
    char *result = NULL;
    mysched_thread_info_t *self_info = NULL;
    size_t result_size = 0;
    if(queue_get(&pool.working, (void **) &self_info) != 0)
        return NULL;  /* Queue empty. */
    result_size = strlen(self_info->name) + (size_t) 1;
    result = (char *) malloc(sizeof(char) * result_size);
    strncpy(result, self_info->name, result_size - 1);
    result[result_size - 1] = '\0';
    return result;
}

// TODO
ssize_t mysched_pwrite(int d, const void *buf, size_t nbytes, off_t offset)
{
    return 0;
}

// TODO
ssize_t mysched_pread(int d, void *buf, size_t nbytes, off_t offset)
{
    return 0;
}

void mysched_go()
{
    int signum = SIGPROF;
    mysched_thread_info_t *clean_info = NULL;

    switch(sigsetjmp(go_jump_point, DONT_SAVE_SIGS)) {
        case 0:
            if(raise(signum) != 0)
                fatal("(EE) Error in raise!\n");
            break;
        default:
            /* Cleaning. */
            while(!queue_is_empty(pool.working)) {
                if(queue_get(&pool.working, (void **) &clean_info) != 0)
                    fatal("Error: getting elements from empty queue!");
                queue_shift_forward(&pool.working);
                queue_rm_before(&pool.working);
                //free(clean_info->name);
                //mysched_stack_free(clean_info->stack);
                //free(clean_info);
            }
            while(!queue_is_empty(pool.waiting)) {
                if(queue_get(&pool.waiting, (void **) &clean_info) != 0)
                    fatal("Error: getting elements from empty queue!");
                queue_shift_forward(&pool.working);
                queue_rm_before(&pool.working);
                //free(clean_info->name);
                //mysched_stack_free(clean_info->stack);
                //free(clean_info);
            }
            // TODO: zdejm tu sigprofa...
    }
    return;
}
