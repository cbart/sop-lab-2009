/** Graph.                                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdlib.h>

/** Weight of `Graph`s edge. */
typedef long weight_t;

/** Graph. */
typedef struct graph
{
    long vertices_quantity;
    weight_t **edge_weights;
} Graph;

/** Inits empty graph (with no edges). */
graph* graph_init(long vertices_quantity);

/** Destroys graph pointed by `g`. */
void graph_destroy(graph *g);

/** Changes given `Edge` in `Graph` pointed by `g`.
 * Return value:
 *   `0` - if edge was inserted properly.
 *  `-1` - if edge is incorrect (e.g. verticle indexes out of bounds). */
int graph_change_edge(graph *g, long edge_from, long edge_to, weight_t weight);

#endif
