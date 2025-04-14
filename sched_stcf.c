#include "scheduler.h"
#include "process.h"
#include <assert.h>
#include <stdlib.h>

typedef struct {
    const struct process **array;
    int size;
    int capacity;
} PriorityQueue;

static PriorityQueue *ready_queue = NULL;
static const struct process *current_proc = NULL;

PriorityQueue *create_queue() {
    PriorityQueue *q = malloc(sizeof(PriorityQueue));
    q->capacity = 10;
    q->size = 0;
    q->array = malloc(q->capacity * sizeof(const struct process *));
    return q;
}

void resize_queue(PriorityQueue *q) {
    q->capacity *= 2;
    q->array = realloc(q->array, q->capacity * sizeof(const struct process *));
}

void push(PriorityQueue *q, const struct process *proc) {
    if (!proc || !proc->current_burst) return;
    if (q->size == q->capacity) resize_queue(q);

    int i = q->size - 1;
    while (i >= 0 && q->array[i]->current_burst->remaining_time > proc->current_burst->remaining_time) {
        q->array[i + 1] = q->array[i];
        i--;
    }
    q->array[i + 1] = proc;
    q->size++;
}

const struct process *peek(PriorityQueue *q) {
    if (!q || q->size == 0) return NULL;
    return q->array[0];
}

void remove_process(PriorityQueue *q, const struct process *proc) {
    if (!q || !proc) return;
    for (int i = 0; i < q->size; i++) {
        if (q->array[i] == proc) {
            for (int j = i; j < q->size - 1; j++) {
                q->array[j] = q->array[j + 1];
            }
            q->size--;
            return;
        }
    }
}

void free_queue(PriorityQueue *q) {
    if (!q) return;
    free(q->array);
    free(q);
}

void maybe_schedule() {
    const struct process *next = peek(ready_queue);
    if (!next || !next->current_burst) return;

    if (!current_proc) {
        if (context_switch(next->pid) == 0) {
            current_proc = next;
            remove_process(ready_queue, next);
        }
    } else if (!current_proc->current_burst || 
               next->current_burst->remaining_time < current_proc->current_burst->remaining_time) {
        if (context_switch(next->pid) == 0) {
            push(ready_queue, current_proc);
            current_proc = next;
            remove_process(ready_queue, next);
        }
    }
}

void sched_init() {
    use_time_slice(FALSE);
    ready_queue = create_queue();
    current_proc = NULL;
}

void sched_new_process(const struct process *proc) {
    assert(proc && proc->state == READY);
    push(ready_queue, proc);
    maybe_schedule();
}

void sched_finished_time_slice(const struct process *proc) {
    (void)proc;  // Not used in STCF
}

void sched_blocked(const struct process *proc) {
    assert(proc && proc->state == BLOCKED);
    if (proc == current_proc) {
        current_proc = NULL;
    }
    remove_process(ready_queue, proc);
    maybe_schedule();
}

void sched_unblocked(const struct process *proc) {
    assert(proc && proc->state == READY);
    push(ready_queue, proc);
    maybe_schedule();
}

void sched_terminated(const struct process *proc) {
    assert(proc && proc->state == TERMINATED);
    if (proc == current_proc) {
        current_proc = NULL;
    }
    remove_process(ready_queue, proc);
    maybe_schedule();
}

void sched_cleanup() {
    free_queue(ready_queue);
    ready_queue = NULL;
    current_proc = NULL;
}
