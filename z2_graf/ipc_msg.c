/** IPC Messaging                                          *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <assert.h>

#include "ipc_msg.h"
#include "customtypes.h"

#define MY_DEFAULT_IPC_KEY 19890630

order_t order_signal()
{
    order_t order;
    order.order_type = 'S';
    return order;
}

order_t order_add_edge(long edge_from, long edge_to, weight_t weight)
{
    order_t order;
    order.order_type = '+';
    order.vertices[0] = edge_from;
    order.vertices[1] = edge_to;
    order.vertices_quantity = 2;
    order.edge_weight = weight;
    return order;
}

order_t order_remove_edge(long edge_from, long edge_to)
{
    order_t order;
    order.order_type = '-';
    order.vertices[0] = edge_from;
    order.vertices[1] = edge_to;
    order.vertices_quantity = 2;
    return order;
}

order_t order_hamiltonian_cycle(long vertices_quantity, long *vertices)
{
    long i;
    order_t order;
    assert(vertices != NULL);
    assert(vertices_quantity <= ORDER_MAX_VERTICES);
    order.order_type = 'H';
    for(i = 0; i < vertices_quantity; i ++)
        order.vertices[i] = vertices[i];
    order.vertices_quantity = vertices_quantity;
    return order;
}

/** Checks if `i` belongs to interval [`a`; `b`)
 * (left-closed, right-opened).
 * Returns:
 *   `TRUE` if `i` belongs to [`a`; `b`)
 *   `FALSE` in other cases. */
bool in_range(long i, long a, long b)
{
    return (a <= i) && (i < b);
}

bool order_is_performable(order_t order, graph *g)
{
    int i;
    long vertices_q;
    assert(g != NULL);
    vertices_q = g->vertices_quantity;
    /* Check if every vertex is a proper value. */
    for(i = 0; i < order.vertices_quantity; i ++)
        if(!in_range(order.vertices[i], 0, g->vertices_quantity))
            return FALSE;
    /* Check proper `vertices_quantity` for every `order_type`. */
    switch(order.order_type) {
        case '+':
            if(order.vertices_quantity != 2)
                return FALSE;
            break;
        case '-':
            if(order.vertices_quantity != 2)
                return FALSE;
            break;
        case 'H':
            break;
        default:
            return FALSE;
    }
    return TRUE;
}

respond_msgbuf make_respond(order_t order, long code)
{
    respond_msgbuf respond;
    respond.msg_type = order.client_pid;
    respond.code = code;
    return respond;
}

void queue_create(orders_queue *q)
{
    assert(q != NULL);
    q->front = NULL;
    q->back = NULL;
}

bool queue_empty(orders_queue *q)
{
    assert(q != NULL);
    return (q->front == NULL);
}

int queue_push(orders_queue *q, order_t order)
{
    queue_node *new_node;
    assert(q != NULL);
    if((new_node = (queue_node *) malloc(sizeof(queue_node))) == NULL)
        return -1;
    new_node->order = order;
    new_node->next = NULL;
    if(queue_empty(q)) {
        q->front = new_node;
        q->back = new_node;
    }
    else {
        q->back->next = new_node;
        q->back = new_node;
    }
    return 0;
}

void queue_pop(orders_queue *q, order_t *order)
{
    queue_node *front_node;
    assert(q != NULL);
    assert(order != NULL);
    if(!queue_empty(q)) {
        front_node = q->front;
        *order = front_node->order;
        q->front = front_node->next;
        if(q->front == NULL)
            q->back = NULL;
        free(front_node);
    }
}

void queue_clear(orders_queue *q)
{
    order_t tmp_order;
    assert(q != NULL);
    while(!queue_empty(q))
        queue_pop(q, &tmp_order);
}

key_t get_ipc_key()
{
    //char *dir = "/tmp";
    key_t ipc_key = MY_DEFAULT_IPC_KEY;
    //if((ipc_key = ftok(dir, CHAR_FLAG)) == (key_t) -1)
    //    return -1;
    return ipc_key;
}
