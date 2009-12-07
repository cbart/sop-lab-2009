/** Multi-threaded server.                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#define _MULTI_THREADED

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <pthread.h>
#include <signal.h>
#include <sys/time.h>
#include <errno.h>

#include "customtypes.h"
#include "err.h"
#include "ipc_msg.h"
#include "thread_pool.h"
#include "graph.h"

/* We have to use `sleep` at the end of the main thread.
 * So to prevent `longjmp` while sleeping (if a signal would
 * come while sleeping) we use the global `signal_occurence`. */
volatile sig_atomic_t signal_occurence = 0;

/* Handling signal. */
void signal_handler(int sig_num)
{
    if(signal_occurence == 0) {
        signal_occurence = 1;
    }
}

/* Thread argument structure. */
typedef struct thread_info_t
{
    thread_pool *pool;             /* Thread pool. */
    pthread_mutex_t *pool_sem;     /* Thread pool semaphore. */
    orders_queue *orders;          /* Orders queue. */
    pthread_mutex_t *orders_sem;   /* Orders queue semaphore. */
    graph *g;                      /* Graph. */
    pthread_rwlock_t* graph_sems;  /* Graph semaphores. */
    int msg_id;                    /* IPC queue id. */
    thread_id id;                  /* This thread id. */
    long wait_abstime;             /* Thread max sleeping time. */
} thread_info_t;

void thread_register(void *arg)
{
    thread_info_t *info = (thread_info_t *) arg;
    info->pool->working[info->id] = TRUE;
}

void thread_unregister(void *arg)
{
    thread_info_t *info = (thread_info_t *) arg;
    if(pthread_mutex_lock(info->pool_sem) != 0)
        syserr("pthread_mutex_lock: While locking thread pool semaphore.");
    info->pool->working[info->id] = FALSE;
    thread_pool_return_thread(info->pool, info->pool->threads + info->id);
    if(pthread_mutex_unlock(info->pool_sem) != 0)
        syserr("pthread_mutex_unlock: While unlocking thread pool sem.");
}

void thread_sleep(void *arg)
{
    struct timespec ts;
    struct timeval tv;
    int timedwait_result;
    thread_info_t *info = (thread_info_t *) arg;

    gettimeofday(&tv, NULL);
    ts.tv_sec = tv.tv_sec;
    ts.tv_nsec = tv.tv_usec * 1000;
    ts.tv_sec += info->wait_abstime;

    if(info->wait_abstime > 0) {
        if(pthread_mutex_unlock(info->orders_sem) != 0)
            syserr("pthread_mutex_unlock: While unlocking orders semaphore.");
        if(pthread_mutex_lock(info->pool_sem) != 0)
            syserr("pthread_mutex_lock: While locking thread pool sem.");
        thread_pool_thread_sleep(info->pool);
        timedwait_result = pthread_cond_timedwait(&info->pool->sleeping,
                info->pool_sem, &ts);
        thread_pool_thread_awoken(info->pool);
        if(pthread_mutex_unlock(info->pool_sem) != 0)
            syserr("pthread_mutex_lock: While unlocking thread pool sem.");
        if(pthread_mutex_lock(info->orders_sem) != 0)
            syserr("pthread_mutex_lock: While locking orders semaphore.");
        if(timedwait_result != 0 && timedwait_result != ETIMEDOUT)
            syserr("phtread_cond_timedwait: in thread while waiting.");
    }
}

int cmp(const void *a_ptr, const void *b_ptr)
{
    long a = *((const long *) a_ptr);
    long b = *((const long *) b_ptr);
    if(a < b)
        return -1;
    else if(a == b)
        return 0;
    else  /* a > b */
        return 1;
}

/* Thread execution function. */
void * server_thread(void *arg)
{
    thread_info_t *info = (thread_info_t *) arg;
    order_t order;
    respond_msgbuf respond;
    thread_register((void *) info);
    pthread_cleanup_push(thread_unregister, (void *) info);
    int i;
    if(pthread_mutex_lock(info->orders_sem) != 0)
        syserr("pthread_mutex_lock: While taking orders queue semaphore.");
    while(!queue_empty(info->orders)) {
        while(!queue_empty(info->orders)) {
            queue_pop(info->orders, &order);
            if(pthread_mutex_unlock(info->orders_sem) != 0)
                syserr("pthread_mutex_unlock: While releasing orders queue "
                        "semaphore.");
            if(!order_is_performable(order, info->g)) {
                fprintf(stderr, "INFO: Order not performable.\n");
                respond = make_respond(order, -1);
            }
            else {
                switch(order.order_type) {
                    case '+':
                        if(order.vertices[0] < order.vertices[1]) {
                            if(pthread_rwlock_wrlock(info->graph_sems +
                                        order.vertices[0]) != 0)
                                syserr("pthread_rwlock_wrlock: While adding "
                                        "edge.");
                            if(pthread_rwlock_wrlock(info->graph_sems +
                                        order.vertices[1]) != 0)
                                syserr("pthread_rwlock_wrlock: While adding "
                                        "edge.");
                        }
                        else if(order.vertices[0] > order.vertices[1]) {
                            if(pthread_rwlock_wrlock(info->graph_sems +
                                        order.vertices[1]) != 0)
                                syserr("pthread_rwlock_wrlock: While adding "
                                        "edge.");
                            if(pthread_rwlock_wrlock(info->graph_sems +
                                        order.vertices[0]) != 0)
                                syserr("pthread_rwlock_wrlock: While adding "
                                        "edge.");
                        }
                        else {
                            if(pthread_rwlock_wrlock(info->graph_sems +
                                        order.vertices[0]) != 0)
                                syserr("pthread_rwlock_wrlock: While adding "
                                        "edge.");
                        }
                        respond = make_respond(order,
                                graph_change_edge(info->g,
                                    order.vertices[0],
                                    order.vertices[1],
                                    order.edge_weight));
                        if(pthread_rwlock_unlock(info->graph_sems
                                    + order.vertices[0]) != 0)
                            syserr("pthread_rwlock_unlock: While adding "
                                    "edge.");
                        if(order.vertices[0] != order.vertices[1]) {
                            if(pthread_rwlock_unlock(info->graph_sems
                                        + order.vertices[1]) != 0)
                                syserr("pthread_rwlock_unlock: While adding "
                                        "edge.");
                        }
                        break;
                    case '-':
                        if(order.vertices[0] < order.vertices[1]) {
                            if(pthread_rwlock_wrlock(info->graph_sems
                                        + order.vertices[0]) != 0)
                                syserr("pthread_rwlock_wrlock: While removing "
                                        "edge.");
                            if(pthread_rwlock_wrlock(info->graph_sems
                                        + order.vertices[1]) != 0)
                                syserr("pthread_rwlock_wrlock: While removing "
                                        "edge.");
                        }
                        else if(order.vertices[0] > order.vertices[1]) {
                            if(pthread_rwlock_wrlock(info->graph_sems
                                        + order.vertices[1]) != 0)
                                syserr("pthread_rwlock_wrlock: While removing "
                                        "edge.");
                            if(pthread_rwlock_wrlock(info->graph_sems
                                        + order.vertices[0]) != 0)
                                syserr("pthread_rwlock_wrlock: While removing "
                                        "edge.");
                        }
                        else {
                            if(pthread_rwlock_wrlock(info->graph_sems
                                        + order.vertices[0]) != 0)
                                syserr("pthread_rwlock_wrlock: While removing "
                                        "edge.");
                        }
                        respond = make_respond(order,
                                1 - graph_change_edge(info->g,
                                    order.vertices[0],
                                    order.vertices[1],
                                    0));
                        if(pthread_rwlock_unlock(info->graph_sems
                                    + order.vertices[0]) != 0)
                            syserr("pthread_rwlock_unlock: While removing "
                                    "edge.");
                        if(order.vertices[0] != order.vertices[1]) {
                            if(pthread_rwlock_unlock(info->graph_sems
                                        + order.vertices[1]) != 0)
                                syserr("pthread_rwlock_unlock: While removing "
                                        "edge.");
                        }
                        break;
                    case 'H':
                        qsort(order.vertices, (size_t) order.vertices_quantity,
                                sizeof(long), cmp);
                        for(i = 0; i < order.vertices_quantity; i++)
                            if(pthread_rwlock_rdlock(info->graph_sems +
                                        order.vertices[i]) != 0)
                                syserr("pthread_rwlock_rdlock: While taking "
                                        "graph semaphore.");
                        respond = make_respond(order,
                                graph_hamiltonian_cost(info->g,
                                    order.vertices_quantity,
                                    order.vertices));
                        for(i = 0; i < order.vertices_quantity; i ++)
                            if(pthread_rwlock_unlock(info->graph_sems +
                                        order.vertices[i]) != 0)
                                syserr("pthread_rwlock_rdlock: While unlocking "
                                        "graph semaphore.");
                        break;
                    default:
                        fatal("(!!) internal error in executing order.");
                }
            }
            if(msgsnd(info->msg_id, &respond,
                        sizeof(respond) - sizeof(long), 0) != 0)
                syserr("msgsnd: While sending error respond.");
            if(pthread_mutex_lock(info->orders_sem) != 0)
                syserr("pthread_mutex_lock: While taking orders queue "
                        "semaphore.");
        }
        thread_sleep((void *) info);
    }
    if(pthread_mutex_unlock(info->orders_sem) != 0)
        syserr("pthread_mutex_unlock: While releasing orders queue "
                "semaphore.");

    pthread_cleanup_pop(TRUE);  /* thread_unregister(info); */

    return (void *) 0;
}

int main(int argc, char **argv)
{
    /* DECLARATION. */

    int i;  /* for the loops. */
    size_t new_thread_id;
    FILE *stdlog = stderr;

    struct sigaction signal_action;
    struct sigaction old_action_int, old_action_hup, old_action_term;

    /* Variables for synchronization. */
    pthread_mutexattr_t mutex_attr;
    pthread_rwlockattr_t rwlock_attr;

    /* Variables covering the threads. */
    long max_running_threads;
    thread_pool pool;
    pthread_mutex_t pool_mutex;
    pthread_t *new_thread;
    pthread_attr_t def_attr;
    void * (* thread_function)(void *) = server_thread;
    thread_info_t* thread_info;
    long wait_abstime;

    /* Variables covering IPC messaging. */
    int msg_id;
    order_msgbuf buffer;
    size_t buf_size = sizeof(order_t);
    key_t ipc_key = get_ipc_key();

    /* Variables covering orders. */
    orders_queue orders_to_execute;
    pthread_mutex_t orders_mutex;

    /* Variables covering graph. */
    size_t vertices_quantity;
    graph g;
    pthread_rwlock_t *graph_sems;

    /* PROGRAM ARGUMENTS. */

    if(argc == 4) {
        vertices_quantity = atoi(argv[1]);
        max_running_threads = atoi(argv[2]);
        wait_abstime = atoi(argv[3]);
    }
    else
        return -1;

    /* INITIALIZATION. */

    /* Signals handlers init. */
    signal_action.sa_handler = signal_handler;
    sigemptyset(&(signal_action.sa_mask));
    signal_action.sa_flags = 0;

    sigaction(SIGINT, &signal_action, &old_action_int);
    sigaction(SIGHUP, &signal_action, &old_action_hup);
    sigaction(SIGTERM, &signal_action, &old_action_term);

    /* Create mutex and rwlock attribute. */
    fprintf(stdlog, "INFO: Creating mutex and rwlock attributes.\n");
    if(pthread_mutexattr_init(&mutex_attr) != 0)
        syserr("pthread_mutexattr_init: While initializing mutexes' attr.");
    if(pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK) != 0)
        syserr("pthread_mutexattr_settype: While setting mutexattr's type.");
    if(pthread_rwlockattr_init(&rwlock_attr) != 0)
        syserr("pthread_rwlockattr_init: While initializing rwlock attr.");

    /* Thread attribute initialization. */
    fprintf(stdlog, "INFO: Creating thread attributes.\n");
    if(pthread_attr_init(&def_attr) != 0)
        syserr("pthread_attr_init: While creating default pthread attributes.");
    if(pthread_attr_setdetachstate(&def_attr, PTHREAD_CREATE_JOINABLE) != 0)
        syserr("pthread_attr_setdetachstate: "
                "While setting default detach state.");

    /* Create thread pool. */
    fprintf(stdlog, "INFO: Creating thread pool.\n");
    if(pthread_mutex_init(&pool_mutex, &mutex_attr) != 0)
        syserr("pthread_mutex_init: While initializing mutex for thread pool.");
    if(thread_pool_create(&pool, max_running_threads) != 0)
        fatal("thread_pool_create: While creating server thread pool.");

    /* Create IPC queue. */
    fprintf(stdlog, "INFO: Creating IPC queue.\n");
    if((msg_id = msgget(ipc_key, 0666 | IPC_CREAT | IPC_EXCL)) == -1)
        syserr("msgget: While connecting to IPC.");
    fprintf(stdlog, "INFO: Getting messages from ipc of key: %d, "
            "with id: %d.\n", ipc_key, msg_id);

    /* Create graph. */
    fprintf(stdlog, "INFO: Creating graph.\n");
    if(graph_init(&g, vertices_quantity) != 0)
        fatal("graph_init: While creating graph.");
    if((graph_sems = (pthread_rwlock_t *) malloc(vertices_quantity *
            sizeof(pthread_rwlock_t))) == NULL)
        syserr("malloc: While allocating space for graph's mutexes.");
    for(i = 0; i < vertices_quantity; i ++)
        if(pthread_rwlock_init(graph_sems + i, &rwlock_attr) != 0)
            syserr("pthread_mutex_init: While initializing mutex.");

    /* Create orders queue. */
    fprintf(stdlog, "INFO: Creating orders queue.\n");
    if(pthread_mutex_init(&orders_mutex, &mutex_attr) != 0)
        syserr("pthread_mutex_init: While creating orders queue mutex.");
    queue_create(&orders_to_execute);

    /* SETUP. */

    /* Set information for threads. */

    thread_info = (thread_info_t *) calloc((size_t) max_running_threads,
            sizeof(thread_info_t));
    thread_info[0].pool = &pool;
    thread_info[0].pool_sem = &pool_mutex;
    thread_info[0].orders = &orders_to_execute;
    thread_info[0].orders_sem = &orders_mutex;
    thread_info[0].g = &g;
    thread_info[0].graph_sems = graph_sems;
    thread_info[0].msg_id = msg_id;
    thread_info[0].id = 0;
    thread_info[0].wait_abstime = wait_abstime;
    for(i = 1; i < max_running_threads; i ++) {
        thread_info[i] = thread_info[0];
        thread_info[i].id = i;
    }

    /* MAIN LOOP. */
    while(signal_occurence == 0) {
        /* Get request from IPC. */
        if(msgrcv(msg_id, &buffer, buf_size, IPC_ORDERS_RESERVED, 0) <= 0) {
            if(errno == EINTR) {
                fprintf(stdlog, "INFO: Signal occured. Aborting.\n");
                break;
            }
            else
                syserr("msgrcv: While receiving IPC message.");
        }
        fprintf(stdlog, "INFO: Order received.\n");
        /* Store order in orders queue. */
        if(pthread_mutex_lock(&orders_mutex) != 0)
            syserr("pthread_mutex_lock: While taking orders queue semaphore "
                    "in the main thread.");
        if(queue_push(&orders_to_execute, buffer.order) != 0)
            fatal("queue_push: While pushing new order into orders queue.");
        if(pthread_mutex_unlock(&orders_mutex) != 0)
            syserr("pthread_mutex_unlock: While unlocking orders queue "
                    "semaphore in the main thread.");
        if(pthread_mutex_lock(&pool_mutex) != 0)
            syserr("pthread_mutex_lock: While locking pool mutex "
                    "in the main thread.");
        if(thread_pool_wakeup_waiting(&pool) != 0) {
            if((new_thread = thread_pool_get_free(&pool)) != NULL) {
                fprintf(stdlog, "INFO: Creating new thread.\n");
                new_thread_id = new_thread - pool.threads;
                if(pthread_create(new_thread, &def_attr, thread_function,
                            (void *) (thread_info + new_thread_id)) != 0)
                    syserr("pthread_create: While creating "
                            "new execution thread.");
            }
            else
                fprintf(stdlog, "INFO: Out of threads.\n");
        }
        else
            fprintf(stdlog, "INFO: Thread signaled.\n");
        if(pthread_mutex_unlock(&pool_mutex) != 0)
            syserr("pthread_mutex_lock: While unlocking pool mutex "
                    "in the main thread.");
    }

    fprintf(stdlog, "INFO: Clearing orders queue.\n");
    if(pthread_mutex_lock(&orders_mutex) != 0)
        syserr("pthread_mutex_lock: While locking orders semaphore.");
    queue_clear(&orders_to_execute);  /* In order to stop the threads. */
    if(pthread_mutex_unlock(&orders_mutex) != 0)
        syserr("pthread_mutex_unlock: While unlocking orders semaphore.");

    for(i = 0; i < max_running_threads; i ++)
        if(pool.working[i])
            if(pthread_join(pool.threads[i], NULL) != 0)
                syserr("pthread_join: While waiting for executor threads.");
    if(signal_occurence != 0)
        sleep(1);
    if(pthread_mutex_destroy(&orders_mutex) != 0)
        syserr("pthread_mutex_destroy: While destroying orders queue mutex.");

    /* CLEANING. */

    /* Destroy the graph. */
    fprintf(stdlog, "INFO: Destroying graph.\n");
    for(i = vertices_quantity - 1; i >= 0; i --)
        if(pthread_rwlock_destroy(graph_sems + i) != 0)
            syserr("pthread_rwlock_destroy: While destroying graph's mutexes.");
    fprintf(stdlog, "INFO: Freeing graph semaphores.\n");
    free(graph_sems);
    fprintf(stdlog, "INFO: Destroying the graph.\n");
    graph_destroy(&g);

    /* Remove IPC queue. */
    fprintf(stdlog, "INFO: Removing IPC queue.\n");
    if(msgctl(msg_id, IPC_RMID, NULL) != 0)
        syserr("msgctl: While removing IPC message queue.");

    /* Destroy thread pool. */
    fprintf(stdlog, "INFO: Destroying thread pool.\n");
    if(thread_pool_destroy(&pool) != 0)
        fatal("thread_pool_destroy: While destroying thread pool.");
    if(pthread_mutex_destroy(&pool_mutex) != 0)
        syserr("pthread_mutex_destroy: While destroying thread pool mutex.");
    free(thread_info);

    /* Destroy thread default attribute. */
    fprintf(stdlog, "INFO: Destroying thread default attribute.\n");
    if(pthread_attr_destroy(&def_attr) != 0)
        syserr("pthread_attr_destroy: "
                "While destroying default pthread attributes");

    /* Destroy mutex and rwlock attribute. */
    fprintf(stdlog, "INFO: Destroying mutex and rwlock attribute.\n");
    if(pthread_rwlockattr_destroy(&rwlock_attr) != 0)
        syserr("pthread_rwlockattr_destroy: While destroying rwlocks' attr.");
    if(pthread_mutexattr_destroy(&mutex_attr) != 0)
        syserr("pthread_mutexattr_destroy: While destroying mutexes' attr.");

    return 0;
}

