/** Graph.                                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "customtypes.h"
#include "err.h"
#include "graph.h"

#define INFTY 1000000

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

typedef struct int_stack_t
{
    long i;
    struct int_stack_t *next;
} int_stack_t;

int push(int_stack_t **stack, long i)
{
    assert(stack != NULL);
    int_stack_t *new_el;
    if((new_el = (int_stack_t *) malloc(sizeof(int_stack_t))) == NULL)
        return -1;
    new_el->next = *stack;
    new_el->i = i;
    *stack = new_el;
    return 0;
}

bool empty(int_stack_t *stack)
{
    return stack == NULL;
}

int pop(int_stack_t **stack, long *elem)
{
    int_stack_t *old_el;
    assert(stack != NULL);
    assert(*stack != NULL);
    old_el = *stack;
    *elem = old_el->i;
    *stack = (*stack)->next;
    free(old_el);
    return 0;
}

int_stack_t * make_stack()
{
    return NULL;
}

bool contains(int_stack_t *stack, long elem)
{
    int_stack_t *tmp_stack = stack;
    while(tmp_stack != NULL) {
        if(elem == tmp_stack->i)
            return TRUE;
        else
            tmp_stack = tmp_stack->next;
    }
    return FALSE;
}

long find_next(int_stack_t *stack, long min_inclusive, long max_exclusive)
{
    long i;
    for(i = min_inclusive; i < max_exclusive; i ++)
        if(!contains(stack, i))
            return i;
    return -1;
}

/** Computes cost of a cycle*/
long cycle_cost(graph *g, long vertices_q, long *vertices, long *perm)
{
    long cur_cost;
    long cost = 0;
    int i;
    for(i = 0; i < vertices_q - 1; i ++) {
        cur_cost = edge_cost(g, vertices[perm[i]], vertices[perm[i + 1]]);
        fprintf(stderr, "cost[%ld, %ld] = %ld, ", vertices[perm[i]],
                vertices[perm[i + 1]], cur_cost);
        if(cur_cost == 0)
            return 0;
        else
            cost += cur_cost;
    }
    cur_cost = edge_cost(g, vertices[perm[vertices_q - 1]],
            vertices[perm[0]]);
    fprintf(stderr, "cost[%ld, %ld] = %ld; ",
            vertices[perm[vertices_q - 1]], vertices[perm[0]], cur_cost);
    if(cur_cost == 0)
        return 0;
    else
        cost += cur_cost;
    return cost;
}

/** Computes minimum of `a` and `b` with 0 as infty. */
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
        fprintf(stderr, "Perm: ");
        for(i = 0; i < vertices_q; i ++)
            fprintf(stderr, "%d, ", (int) v_index_perm[i]);
        *cost = min(*cost, cycle_cost(g, vertices_q, vertices, v_index_perm));
        fprintf(stderr, "Min cost: %d.\n", (int) *cost);
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

    fprintf(stderr, "Hamilton on: ");
    for(i = 0; i < vertices_q; ++i) {
        fprintf(stderr, "%ld; ", vertices[i]);
        v_index_perm[i] = i;
    }
    fprintf(stderr, "\n");

    hamiltonian_cost_backtrack(g, vertices_q, vertices, v_index_perm,
            vertices_q - 1, &cost);

    free(v_index_perm);
    return cost;
}
