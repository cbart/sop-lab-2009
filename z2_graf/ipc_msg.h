/** IPC Messaging                                          *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#ifndef _IPC_MSG_H_
#define _IPC_MSG_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "graph.h"
#include "customtypes.h"

#define CHAR_FLAG '*'

/** Messages with types of range [1..IPC_ORDERS_RESERVED]
 * are reserved for Client->Server queries.
 * Server respond will have always type IPC_ORDERS_RESERVED + PID
 * where PID is id of client process. */
#define IPC_ORDERS_RESERVED 1

/** Maximum vertices*/
#define ORDER_MAX_VERTICES 50

/** Order to send via IPC message queue. */
typedef struct order_t
{
    char order_type; /** ['+', '-', 'H']; */
    long vertices[ORDER_MAX_VERTICES];
    long vertices_quantity;
    weight_t edge_weight; /** if order_type == '+' here is the edge weight. */
} order_t;

/** Creates order, which adds edge `edge_from`->`edge_to`
 * with given `weight`. */
order_t order_add_edge(long edge_from, long edge_to, weight_t weight);

/** Creates order, which removes edge `edge_from`->`edge_to`. */
order_t order_remove_edge(long edge_from, long edge_to);

/** Creates order, which cheks hamilton cycle
 * in a subgraph of given vertices array. */
order_t order_hamilton_cycle(long vertices_quantity, long *vertices);

/** Checks if `order` is performable on given graph `g`.
 * Returns:
 *   `FALSE` if order is not performable on given graph.
 *   `TRUE` in other cases. */
bool order_is_performable(order_t order, graph *g);

/** Single node of orders queue. */
typedef struct queue_node
{
    order_t order;
    struct queue_node *next;
} queue_node;

/** Queue of orders. */
typedef struct orders_queue
{
    queue_node *front;
    queue_node *back;
} orders_queue;

/** Creates empty queue with given structure. */
void queue_create(orders_queue *q);

/** Indicates if given orders queue is empty.
 * Returns:
 *   `TRUE` if queue is empty,
 *   `FALSE` otherwise. */
bool queue_empty(orders_queue *q);

/** Pushes given `order` into end of the `q` order queue.
 * Returns:
 *   `0` if everything went ok.
 *   `-1` if an error occured. */
int queue_push(orders_queue *q, order_t order);

/** Pops element from front of `q` and assings it to a structure
 * pointed by `order`.
 * If the queue is empty - does nothing. */
void queue_pop(orders_queue *q, order_t *order);

/** Clears given queue. */
void queue_clear(orders_queue *q);

/** Generates ipc key.
 * Returns:
 *   `-1` in case of an error,
 *   ipc key in other cases. */
key_t get_ipc_key();

#endif
