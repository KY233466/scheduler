#include "scheduler.h"
#include "process.h"
#include <assert.h>
#include <stdlib.h>
#include <stddef.h>

typedef struct {
    const struct process **array;
    int size;
    int capacity;
} PriorityQueue;

static PriorityQueue *ready_queue = NULL;

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
    if (q->size == q->capacity) {
        resize_queue(q);
    }
    
    int i = q->size - 1;
    while (i >= 0 && q->array[i]->current_burst->remaining_time > proc->current_burst->remaining_time) {
        q->array[i + 1] = q->array[i];
        i--;
    }
    q->array[i + 1] = proc;
    q->size++;
}

const struct process *peek(PriorityQueue *q) {
    if (q->size == 0) return NULL;
    return q->array[0];
}

void remove_process(PriorityQueue *q, const struct process *proc) {
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
    free(q->array);
    free(q);
}

void sched_init() {
    use_time_slice(FALSE);
    ready_queue = create_queue();
}

void sched_new_process(const struct process *proc) {
    assert(proc->state == READY);
    push(ready_queue, proc);
    
    const struct process *next = peek(ready_queue);
    pid_t current = get_current_proc();
    const struct process *current_proc = NULL;
    
    // Find current process in queue
    for (int i = 0; i < ready_queue->size; i++) {
        if (ready_queue->array[i]->pid == current) {
            current_proc = ready_queue->array[i];
            break;
        }
    }
    
    // Switch if:
    // 1. No process is running, or
    // 2. New process has shorter remaining time than current
    if (next && (current == -1 || 
                (current_proc && next->current_burst->remaining_time < current_proc->current_burst->remaining_time))) {
        context_switch(next->pid);
    }
}

void sched_finished_time_slice(const struct process *proc) {
    (void)proc;
}

void sched_blocked(const struct process *proc) {
    assert(proc->state == BLOCKED);
    remove_process(ready_queue, proc);
    
    const struct process *next = peek(ready_queue);
    pid_t current = get_current_proc();
    
    // Only switch if:
    // 1. There is a process to switch to
    // 2. It's not the process that just blocked (which should already be removed)
    if (next && next->pid != current) {
        context_switch(next->pid);
    }
}

void sched_unblocked(const struct process *proc) {
    assert(proc->state == READY);
    push(ready_queue, proc);
    
    const struct process *next = peek(ready_queue);
    pid_t current = get_current_proc();
    const struct process *current_proc = NULL;
    
    // Find current process in queue
    for (int i = 0; i < ready_queue->size; i++) {
        if (ready_queue->array[i]->pid == current) {
            current_proc = ready_queue->array[i];
            break;
        }
    }
    
    // Only switch if next process has less remaining time
    if (next && current != next->pid &&
        (!current_proc || next->current_burst->remaining_time < current_proc->current_burst->remaining_time)) {
        context_switch(next->pid);
    }
}

void sched_terminated(const struct process *proc) {
    assert(proc->state == TERMINATED);
    remove_process(ready_queue, proc);
    
    const struct process *next = peek(ready_queue);
    if (next) {
        context_switch(next->pid);
    }
}

void sched_cleanup() {
    free_queue(ready_queue);
}
