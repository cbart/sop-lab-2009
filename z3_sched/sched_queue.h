/** Author: Cezary Bartoszuk <cbart@students.mimuw.edu.pl>
 *  Username: cb277617@students.mimuw.edu.pl
 *  */

#ifndef _SCHED_QUEUE_H_
#define _SCHED_QUEUE_H_

/** Circular queue entry type. */
typedef struct queue_entry
{
    struct queue_entry *prev;
    struct queue_entry *next;
    void *value;
} queue_entry_t;

/** Chechs whether given queue is empty. */
int queue_is_empty(queue_entry_t *q);

/** Creates new queue. */
int queue_init(queue_entry_t **q);

/** Adds element before pointed one in queue. */
int queue_add_before(queue_entry_t **q, void *new_value);

/** Removes element before pointed one in queue. */
int queue_rm_before(queue_entry_t **q);

/** Shifts queue one element forward. */
int queue_shift_forward(queue_entry_t **q);

/** Shifts queue one element backward. */
int queue_shift_backward(queue_entry_t **q);

/** Obtains poiter to stored value. */
int queue_get(queue_entry_t **q, void **value);

#endif  /* _SCHED_QUEUE_H_ */
