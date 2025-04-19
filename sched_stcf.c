#include "scheduler.h"
#include "process.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

typedef struct {
    const struct process **array;
    int size;
    int capacity;
} PriorityQueue;

static PriorityQueue *ready_queue = NULL;
static const struct process *current_proc = NULL;

static PriorityQueue *create_queue() {
    PriorityQueue *q = malloc(sizeof(PriorityQueue));
    assert(q);
    q->capacity = 10;
    q->size = 0;
    q->array = malloc(q->capacity * sizeof(const struct process *));
    assert(q->array);
    return q;
}

static void resize_queue(PriorityQueue *q) {
    q->capacity *= 2;
    q->array = realloc(q->array, q->capacity * sizeof(const struct process *));
    assert(q->array);
}

static void push(PriorityQueue *q, const struct process *proc) {
    assert(proc);
    assert(proc->current_burst);  // Defensive: can't push a proc without bursts

    if (q->size == q->capacity) resize_queue(q);

    int i = q->size - 1;
    while (i >= 0 && q->array[i]->current_burst->remaining_time > proc->current_burst->remaining_time) {
        q->array[i + 1] = q->array[i];
        i--;
    }
    q->array[i + 1] = proc;
    q->size++;
}

static const struct process *peek(PriorityQueue *q) {
    if (!q || q->size == 0) return NULL;
    return q->array[0];
}

static void remove_process(PriorityQueue *q, const struct process *proc) {
    if (!q || !proc) return;
    for (int i = 0; i < q->size; i++) {
        if (q->array[i]->pid == proc->pid) {
            for (int j = i; j < q->size - 1; j++) {
                q->array[j] = q->array[j + 1];
            }
            q->size--;
            return;
        }
    }
}

static void free_queue(PriorityQueue *q) {
    if (!q) return;
    free(q->array);
    free(q);
}

static void schedule_if_needed() {
    if (!ready_queue) {
        fprintf(stderr, "ERROR: ready_queue is NULL!\n");
        exit(1);
    }

    const struct process *next = peek(ready_queue);
    if (!next || !next->current_burst) return;

    pid_t current_pid = get_current_proc();

    // If CPU is idle or our local tracker is NULL or missing a burst
    if (current_pid == -1 || current_proc == NULL || current_proc->current_burst == NULL) {
        if (context_switch(next->pid) == 0) {
            // fprintf(stderr, "(debug) switching to proc %d (CPU idle)\n", next->pid);
            current_proc = next;
            remove_process(ready_queue, next);
        }
    } else {
        const struct process *current = current_proc;

        if (!current->current_burst || next->current_burst->remaining_time < current->current_burst->remaining_time) {
            if (context_switch(next->pid) == 0) {
                // fprintf(stderr, "(debug) preempting proc %d with proc %d\n", current->pid, next->pid);
                if (current->state == READY) {
                    push(ready_queue, current);
                }
                current_proc = next;
                remove_process(ready_queue, next);
            }
        }
    }
}

void sched_init() {
    use_time_slice(FALSE); // STCF is non-time-sliced
    ready_queue = create_queue();
    current_proc = NULL;
}

void sched_new_process(const struct process* proc) {
    assert(proc && proc->state == READY);
    push(ready_queue, proc);
    schedule_if_needed();
}

void sched_finished_time_slice(const struct process* proc) {
    (void)proc; // Unused in STCF
}

void sched_blocked(const struct process* proc) {
    assert(proc && proc->state == BLOCKED);
    if (proc == current_proc) {
        current_proc = NULL;
    }
    remove_process(ready_queue, proc);
    schedule_if_needed();
}

void sched_unblocked(const struct process* proc) {
    assert(proc && proc->state == READY);
    push(ready_queue, proc);
    schedule_if_needed();
}

void sched_terminated(const struct process* proc) {
    assert(proc && proc->state == TERMINATED);
    if (proc == current_proc) {
        current_proc = NULL;
    }
    remove_process(ready_queue, proc);
    schedule_if_needed();
}

void sched_cleanup() {
    free_queue(ready_queue);
    ready_queue = NULL;
    current_proc = NULL;
}
