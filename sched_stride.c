#include "scheduler.h"
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#define STRIDE_CONSTANT 1000000
#define MAX_PID 32768

typedef struct stride_proc {
    const struct process* proc;
    unsigned long stride;
    unsigned long pass;
    struct stride_proc* next;
} stride_proc_t;

static stride_proc_t* ready_list = NULL;
static stride_proc_t* pid_map[MAX_PID];

// Create and register a new stride_proc
static stride_proc_t* create_stride_proc(const struct process* proc) {
    stride_proc_t* sp = (stride_proc_t*)malloc(sizeof(stride_proc_t));
    sp->proc = proc;
    sp->stride = STRIDE_CONSTANT / proc->tickets;
    sp->pass = 0;
    sp->next = NULL;
    pid_map[proc->pid] = sp;
    return sp;
}

// Sorted insert into ready_list by pass value
static void add_to_ready_list(stride_proc_t* sp) {
    if (!ready_list || sp->pass < ready_list->pass) {
        sp->next = ready_list;
        ready_list = sp;
        return;
    }
    stride_proc_t* curr = ready_list;
    while (curr->next && curr->next->pass <= sp->pass) {
        curr = curr->next;
    }
    sp->next = curr->next;
    curr->next = sp;
}

// Remove a process from the ready list
static void remove_from_ready_list(pid_t pid) {
    stride_proc_t** curr = &ready_list;
    while (*curr) {
        if ((*curr)->proc->pid == pid) {
            stride_proc_t* to_remove = *curr;
            *curr = (*curr)->next;
            to_remove->next = NULL;
            return;
        }
        curr = &(*curr)->next;
    }
}

// Only switch if necessary
static void schedule_next() {
    if (!ready_list) return;

    pid_t next = ready_list->proc->pid;
    if (ready_list->proc->state != READY) return;  // ✅ Prevent bad switch

    pid_t current = get_current_proc();
    if (current != next) {
        context_switch(next);
    }
}

// Increment pass for the given process
static void update_pass(pid_t pid) {
    stride_proc_t* sp = pid_map[pid];
    if (sp) {
        sp->pass += sp->stride;
    }
}

void sched_init() {
    use_time_slice(TRUE);
    for (int i = 0; i < MAX_PID; ++i) {
        pid_map[i] = NULL;
    }
}

void sched_new_process(const struct process* proc) {
    assert(READY == proc->state);
    stride_proc_t* sp = create_stride_proc(proc);
    add_to_ready_list(sp);
    if (get_current_proc() == -1) {
        schedule_next();
    }
}

void sched_finished_time_slice(const struct process* proc) {
    pid_t pid = proc->pid;
    stride_proc_t* sp = pid_map[pid];
    if (!sp) return;

    // Advance pass
    update_pass(pid);

    // Remove and re-add to ready list (only if it’s still in CPU_BURST)
    remove_from_ready_list(pid);
    add_to_ready_list(sp);

    schedule_next();
}

void sched_blocked(const struct process* proc) {
    assert(BLOCKED == proc->state);
    update_pass(proc->pid);
    remove_from_ready_list(proc->pid);
    schedule_next();
}

void sched_unblocked(const struct process* proc) {
    assert(READY == proc->state);
    stride_proc_t* sp = pid_map[proc->pid];
    if (!sp) return;

    add_to_ready_list(sp);

    if (get_current_proc() == -1) {
        schedule_next();
    }
}

void sched_terminated(const struct process* proc) {
    assert(TERMINATED == proc->state);
    update_pass(proc->pid);
    remove_from_ready_list(proc->pid);
    free(pid_map[proc->pid]);
    pid_map[proc->pid] = NULL;
    schedule_next();
}

void sched_cleanup() {
    for (int i = 0; i < MAX_PID; ++i) {
        if (pid_map[i]) {
            free(pid_map[i]);
            pid_map[i] = NULL;
        }
    }
    ready_list = NULL;
}
