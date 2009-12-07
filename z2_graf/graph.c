/** Graph.                                                 *
 * author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>  *
 *     id: cb277617@students.mimuw.edu.pl                  */

#include <stdlib.h>
#include <assert.h>
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
        return 1;
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
long cycle_cost(graph *g, long *vertices, int_stack_t *stack)
{
    int_stack_t *tmp = stack;
    long end_elem = tmp->i;
    long cur_cost;
    long cost = 0;
    while(tmp->next != NULL) {
        cur_cost = edge_cost(g, vertices[tmp->i], vertices[tmp->next->i]);
        if(cur_cost == 0)
            return 0;
        else
            cost += cur_cost;
        tmp = tmp->next;
    }
    cur_cost = edge_cost(g, vertices[tmp->i], vertices[end_elem]);
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

/** Changes stack to head to next permutation. */
void next_elem(int_stack_t **stack, long max_exclusive, long *stack_size)
{
    long cur_elem;
    if(*stack_size == max_exclusive) {
        /* fold the stack. */
        do {
            pop(stack, &cur_elem);
            *stack_size = *stack_size - 1;
        } while((*stack) == NULL || cur_elem > (*stack)->i);
        if((*stack) != NULL)
            (*stack)->i ++;
    }
    else {
        /* add to stack. */
        cur_elem = find_next(*stack, (*stack)->i + 1, max_exclusive);
        assert(cur_elem >= 0);
        push(stack, cur_elem);
    }
}

long graph_hamiltonian_cost(graph *g, long vertices_q, long *vertices)
{
    long stack_size = 0;
    int_stack_t *backtrack_stack = make_stack();
    long cost = 0;
    assert(g != NULL);
    assert(vertices != NULL);
    push(&backtrack_stack, 0);
    stack_size ++;
    while(!empty(backtrack_stack)) {
        if(stack_size == vertices_q)
            cost = min(cost, cycle_cost(g, vertices, backtrack_stack));
        next_elem(&backtrack_stack, vertices_q, &stack_size);
    }
    return cost;
}
