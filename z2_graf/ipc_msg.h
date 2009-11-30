/** IPC Messaging                                          *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#ifndef _IPC_MSG_H_
#define _IPC_MSG_H_

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>

#include "graph.h"
#include "order.h"

#define CHAR_FLAG "*"

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
    Weight edge_weight; /** if order_type == '+' here is the edge weight. */
} order_t;

/** Creates order, which adds edge `edge_from`->`edge_to`
 * with given `weight`. */
order_t order_add_edge(long edge_from, long edge_to, weight_t weight);

/** Creates order, which removes edge `edge_from`->`edge_to`. */
order_t order_remove_edge(long edge_from, long edge_to);

/** Creates order, which cheks hamilton cycle
 * in a subgraph of given vertices array. */
order_t order_hamilton_cycle(long vertices_quantity, long *vertices);



/** Generates ipc key.
 * Returns:
 *   `-1` in case of an error,
 *   ipc key in other cases. */
key_t get_ipc_key();

#endif
