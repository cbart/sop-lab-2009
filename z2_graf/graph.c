/** Graph.                                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <stdlib.h>
#include "err.h"
#include "graph.h"
#include "assert.h"

graph* graph_init(long vertices_quantity)
{
    long i;
    graph *new_graph;
    weight_t **new_edges;
    /* Allocates space for `Graph` structure and the edges. */
    if(NULL == (new_graph = (graph *) malloc(sizeof(graph))))
        syserr("graph_init: while allocating Graph in malloc(size_t).");
    if(NULL == (new_edges = (weight_t **) calloc(vertices_quantity,
                    sizeof(weight *))))
        syserr("graph_init: while allocating edges array "
                "(first dimension) in calloc(size_t, size_t).");
    for(i = 0; i < vertices_quantity; i ++)
        if(NULL ==(new_edges[i] = (weight_t *) calloc(vertices_quantity,
                        sizeof(weight_t))))
            syserr("graph_init: while allocating edges array "
                    "(second dimension) in calloc(size_t, size_t)");
    new_graph->vertices_quantity = vertices_quantity;
    new_graph->edge_weights = new_edges;
    return new_graph;
}

void graph_destroy(graph *g)
{
    long i;
    assert(g != NULL);
    assert(g->edge_weights != NULL);
    for(i = g->vertices_quantity - 1; i >= 0; i --) {
        assert(g->edge_weights != NULL);
        free(g->edge_weights[i]);
    }
    free(g->edge_weights);
    free(g);
}

int graph_chenge_edge(graph *g, long edge_from, long edge_to, weight_t weight)
{
    assert(g != NULL);
    if((0 <= edge_from) && (edge_from < g->vertices_quantity)
            && (0 <= edge_to) && (edge_to < g->vertices_quantity)) {
        g->edge_weights[edge_from][edge_to] = weight;
        return 0;
    }
    else
        return -1;
}
