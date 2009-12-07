/** Graph.                                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <stdlib.h>
#include <assert.h>
#include "customtypes.h"
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
}

int graph_change_edge(graph *g, long edge_from, long edge_to, weight_t weight)
{
    int ret_code;
    assert(g != NULL);
    if((0 <= edge_from) && (edge_from < g->vertices_quantity)
            && (0 <= edge_to) && (edge_to < g->vertices_quantity)) {
        if(g->edge_weights[edge_from][edge_to] != 0)
            ret_code = 1;
        else
            ret_code = 0;
        g->edge_weights[edge_from][edge_to] = weight;
        return ret_code;
    }
    else
        return -1;
}

long edge_cost(graph *g, long v_from, long v_to)
{
    assert(g != NULL);
    return g->edge_weights[v_from][v_to];
}

/** Computes cost of a cycle*/
long cycle_cost(graph *g, long vertices_q, long *vertices, long *perm)
{
    long cur_cost;
    long cost = 0;
    int i;
    for(i = 0; i < vertices_q - 1; i ++) {
        cur_cost = edge_cost(g, vertices[perm[i]], vertices[perm[i + 1]]);
        if(cur_cost == 0)
            return 0;
        else
            cost += cur_cost;
    }
    cur_cost = edge_cost(g, vertices[perm[vertices_q - 1]],
            vertices[perm[0]]);
    if(cur_cost == 0)
        return 0;
    else
        cost += cur_cost;
    return cost;
}

/** Computes minimum of `a` and `b`.
 * Treats `0` as infty. */
long min(long a, long b)
{
    /* 0 is infty here. */
    if(a == 0)
        return b;
    else if(b == 0)
        return a;
    else
        return (a < b) ? a : b;
}

/** Back-tracking function for computing hamiltonian cost of given subgraph. */
void hamiltonian_cost_backtrack(graph *g, long vertices_q, long *vertices,
        long *v_index_perm, long vertex_index, long *cost)
{
    int i;
    long temp_exchange;
    assert(g != NULL);
    assert(vertices != NULL);
    assert(v_index_perm != NULL);
    assert(cost != NULL);
    if(vertex_index > 0) {
        hamiltonian_cost_backtrack(g, vertices_q, vertices, v_index_perm,
                vertex_index - 1, cost);
        for(i = 0; i < vertex_index; i ++ ) {
            /* Exchange. */
            temp_exchange = v_index_perm[i];
            v_index_perm[i] = v_index_perm[vertex_index];
            v_index_perm[vertex_index] = temp_exchange;
            /* Compute hamiltonian cost. */
            hamiltonian_cost_backtrack(g, vertices_q, vertices, v_index_perm,
                    vertex_index - 1, cost);
            /* Exchange backwards. */
            temp_exchange = v_index_perm[i];
            v_index_perm[i] = v_index_perm[vertex_index];
            v_index_perm[vertex_index] = temp_exchange;
        }
    }
    else {  /* vertex_index == 0 */
        *cost = min(*cost, cycle_cost(g, vertices_q, vertices, v_index_perm));
    }
}

long graph_hamiltonian_cost(graph *g, long vertices_q, long *vertices)
{
    long *v_index_perm;
    int i;
    long cost = 0;

    assert(g != NULL);
    assert(vertices != NULL);

    if((v_index_perm = (long *) malloc(vertices_q * sizeof(long))) == NULL)
        return -1;

    for(i = 0; i < vertices_q; ++i)
        v_index_perm[i] = i;

    hamiltonian_cost_backtrack(g, vertices_q, vertices, v_index_perm,
            vertices_q - 1, &cost);

    free(v_index_perm);
    return cost;
}
