/** IPC Messaging                                          *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <assert.h>

#include "ipc_msg.h"
#include "order.h"

order_t order_add_edge(long edge_from, long edge_to, weight_t weight)
{
    order_t order;
    order.order_type = '+';
    order.vertices[0] = edge_from;
    order.vertices[1] = edge_to;
    order.edge_weight = weight;
    return order;
}

order_t order_remove_edge(long edge_from, long edge_to)
{
    ordert_t order;
    order.order_type = '-';
    order.vertices[0] = edge_from;
    order.vertices[1] = edge_to;
    return order;
}

order_t order_hamilton_cycle(long vertices_quantity, long *vertices)
{
    long i;
    order_t order;
    assert(vertices != NULL);
    assert(vertices_quantity <= ORDER_MAX_VERTICES);
    order.order_type = 'H';
    for(i = 0; i < vertices_quantity; i ++)
        order.vertices[i] = vertices[i];
}

key_t get_ipc_key()
{
    long dir_string_size;
    char *dir;
    char *buf;
    key_t ipc_key;
    dir_string_size = pathconf("/tmp", _PC_PATH_MAX);
    if((buf = (char *) malloc((size_t) dir_string_size)) == NULL)
        return -1;
    if((dir = getcwd(buf, (size_t) dir_string_size)) == NULL)
        return -1;
    if((ipc_key = ftok(dir, CHAR_FLAG)) == (key_t) -1)
        return -1;
    free(buf);
    return ipc_key;
}
