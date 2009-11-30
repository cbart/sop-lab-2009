/** author: Cezary Bartoszuk
 *  e-mail: cbart@students.mimuw.edu.pl
 *      id: cb277617 */

#include <stdlib.h>
#include "err.h"
#include "graph.h"
#include "assert.h"

Verticle initVert(Integer index)
{
    /* In current implementation `Verticle` is simply its index.
     * But it may not in the future, so please use `initVert`
     * for creating a verticle. */
    return index;
}

Integer getIndex(Verticle v)
{
    return v;
}

Edge initEdge(Integer from_index, Integer to_index, Weight weight)
{
    Edge e;
    e.from = from_index;
    e.to = to_index;
    e.weight = weight;
    return e;
}

Graph* initGraph(Integer verticles_quantity)
{
    Integer i;
    Graph *new_graph;
    Weight **new_edges;
    /* Allocates space for `Graph` structure and the edges. */
    if(NULL == (new_graph = (Graph *) malloc(sizeof(Graph))))
        syserr("initGraph: while allocating Graph in malloc(size_t).");
    if(NULL == (new_edges = (Weight **) calloc(verticles_quantity,
                    sizeof(Weight *))))
        syserr("initGraph: while allocating edges array "
                "(first dimension) in calloc(size_t, size_t).");
    for(i = 0; i < verticles_quantity; i ++)
        if(NULL ==(new_edges[i] = (Weight *) calloc(verticles_quantity,
                        sizeof(Weight))))
            syserr("initGraph: while allocating edges array "
                    "(second dimension) in calloc(size_t, size_t)");
    new_graph->verticles_quantity = verticles_quantity;
    new_graph->edge_weights = new_edges;
    return new_graph;
}

void destroyGraph(Graph *q)
{
    Integer i;
    assert(q != NULL);
    assert(q->edge_weights != NULL);
    for(i = q->verticles_quantity - 1; i >= 0; i --) {
        assert(q->edge_weights != NULL);
        free(q->edge_weights[i]);
    }
    free(q->edge_weights);
    free(q);
}

int changeEdge(Graph *g, Edge e)
{
    assert(g != NULL);
    if((0 <= e.from) && (e.from < g->verticles_quantity)
            && (0 <= e.to) && (e.to < g->verticles_quantity)) {
        g->edge_weights[e.from][e.to] = e.weight;
        return 0;
    }
    else
        return -1;
}
