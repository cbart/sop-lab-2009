/** Author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>
 *  Username: cb277617@students.mimuw.edu.pl
 *  */

#include <stdlib.h>
#include "sched_queue.h"

int queue_is_empty(queue_entry_t *q)
{
    return q == NULL;
}

int queue_init(queue_entry_t **q)
{
    *q = NULL;
    return 0;
}

int queue_add_before(queue_entry_t **q, void *new_value)
{
    queue_entry_t *new_entry;
    if((new_entry = (queue_entry_t *) malloc(sizeof(queue_entry_t))) == NULL)
        return -1;
    if(queue_is_empty(*q)) {
        new_entry->prev = new_entry;
        new_entry->next = new_entry;
        new_entry->value = new_value;
        *q = new_entry;
    }
    else {
        new_entry->prev = (*q)->prev;
        new_entry->next = *q;
        new_entry->value = new_value;
        new_entry->prev->next = new_entry;
        new_entry->next->prev = new_entry;
    }
    return 0;
}

int queue_rm_before(queue_entry_t **q)
{
    queue_entry_t *tmp = NULL;
    if(queue_is_empty(*q))
        return 1;
    if((*q)->next == (*q)) {
        free(*q);
        *q = NULL;
    }
    else {
        tmp = (*q)->prev;
        tmp->prev->next = (*q);
        (*q)->prev = tmp->prev;
        free(tmp);
    }
    return 0;
}

int queue_shift_forward(queue_entry_t **q)
{
    if(queue_is_empty(*q))
        return 1;
    (*q) = (*q)->next;
    return 0;
}

int queue_shift_backward(queue_entry_t **q)
{
    if(queue_is_empty(*q))
        return 1;
    (*q) = (*q)->prev;
    return 0;
}

int queue_get(queue_entry_t **q, void **value)
{
    if(queue_is_empty(*q))
        return 1;
    *value = (*q)->value;
    return 0;
}
