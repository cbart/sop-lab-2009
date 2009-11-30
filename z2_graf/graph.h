/** author: Cezary Bartoszuk
 *  e-mail: cbart@students.mimuw.edu.pl
 *      id: cb277617 */

#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <stdlib.h>

//TODO: znajdź dobre miejsce na tę deklarację...
typedef long Integer;

/** Verticle in a `Graph`. */
typedef Integer Verticle;

/** Inits verticle of given index. */
Verticle initVert(Integer index);

/** Extracts index from `v`. */
Integer getIndex(Verticle v);

/** Weight of `Graph`s edge. */
typedef long Weight;

/** Edge in a `Graph`. */
typedef struct Edge
{
    Integer from;
    Integer to;
    Weight weight;
} Edge;

/** Inits edge with given `Verticle` indexes and given `weight`. */
Edge initEdge(Integer from_index, Integer to_index, Weight weight);

/** Graph. */
typedef struct Graph
{
    Integer verticles_quantity;
    Weight **edge_weights;
} Graph;

/** Inits empty graph (with no edges). */
Graph* initGraph(Integer edges_quantity);

/** Destroys graph pointed by `g`. */
void destroyGraph(Graph *g);

/** Changes given `Edge` in `Graph` pointed by `g`.
 * Return value:
 *   `0` - if edge was inserted properly.
 *  `-1` - if edge is incorrect (e.g. verticle indexes out of bounds). */
int changeEdge(Graph *g, Edge e);

#endif
