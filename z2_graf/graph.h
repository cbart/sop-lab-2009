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
} graph;

/** Inits empty graph (with no edges).
 * Returns:
 *   `-1` when an error occured,
 *   `0` otherwise. */
int graph_init(graph *new_graph, long vertices_quantity);

/** Destroys graph pointed by `g`. */
void graph_destroy(graph *g);

/** Changes given `Edge` in `Graph` pointed by `g`.
 * Return value:
 *   `0` - if edge was inserted (before there was no edge).
 *   `1` - if edge was changed (there was edge before).
 *   `-1` - if edge is incorrect (e.g. verticle indexes out of bounds). */
int graph_change_edge(graph *g, long edge_from, long edge_to, weight_t weight);

/** Computes minimal cost of hamiltonian cycle. */
long graph_hamiltonian_cost(graph *g, long vertices_q, long* vertices);

#endif  /* _GRAPH_H_ */
