/** Graph.                                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <stdlib.h>
#include <assert.h>
#include "err.h"
#include "graph.h"

int graph_init(graph *new_graph, long vertices_quantity)
{
    long i;
    weight_t **new_edges;
    /* Allocates space for `Graph` structure and the edges. */
    if(NULL == (new_edges = (weight_t **) calloc(vertices_quantity,
                    sizeof(weight_t *))))
        return -1;
    for(i = 0; i < vertices_quantity; i ++)
        if(NULL ==(new_edges[i] = (weight_t *) calloc(vertices_quantity,
                        sizeof(weight_t)))) {
            //TODO: dealokacja zasobów z malloca powyżej;
            return -1;
        }
    new_graph->vertices_quantity = vertices_quantity;
    new_graph->edge_weights = new_edges;
    return 0;
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

long graph_hamilton_cost(graph *g, long verticles_q, long *verticles)
{
    assert(g != NULL);
    assert(verticles != NULL);
    
}
