#include "scheduler.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct {
  const struct process **array;
  int front;
  int rear;
  int size;
  int capacity;
} Queue;

static Queue *queue = NULL;

Queue *createQueue() {
  Queue *queue = (Queue *)malloc(sizeof(Queue));
  queue->capacity = 10;
  queue->front = 0;
  queue->size = 0;
  queue->rear = -1;
  queue->array = (const struct process **)malloc(queue->capacity * sizeof(const struct process *));
  
  return queue;
}

int isFull() {
  return (queue->size == queue->capacity);
}

int isEmpty() {
  return (queue->size == 0);
}

int hasOne() {
  return (queue->size == 1);
}

void resizeQueue() {
  int newCapacity = queue->capacity * 2;
  const struct process **newArray = (const struct process **)malloc(newCapacity * sizeof(const struct process *));

  for (int i = 0; i < queue->size; i++)
  {
    newArray[i] = queue->array[(queue->front + i) % queue->capacity];
  }

  free(queue->array);
  queue->array = newArray;
  queue->front = 0;
  queue->rear = queue->size - 1;
  queue->capacity = newCapacity;
}

void push(const struct process* item)
{
  if (isFull(queue))
  {
    resizeQueue(queue);
  }
  queue->rear = (queue->rear + 1) % queue->capacity;
  queue->array[queue->rear] = item;
  queue->size++;
}

const struct process* first() {
  return queue->array[queue->front];
}

const struct process* pop() {
  const struct process *item = queue->array[queue->front];
  queue->front = (queue->front + 1) % queue->capacity;
  queue->size--;
  return item;
}

void freeQueue() {
  free(queue->array);
  free(queue);
}

/*************************
 * ROUND ROBIN Scheduler *
 *************************/

/* sched_init
 *   will be called exactly once before any processes arrive or any other events
 */
void sched_init() {
  use_time_slice(TRUE);
  queue = createQueue();
}


/* sched_new_process
 *   will be called when a new process arrives (i.e., fork())
 *
 * proc - the new process that just arrived
 */
void sched_new_process(const struct process* proc) {
  assert(READY == proc->state);
  // printf("in sched_new_process\n");
  push(proc);
  if (hasOne())
  {
    context_switch(proc->pid);
  }
}


/* sched_finished_time_slice
 *   will be called when the currently running process finished a time slice
 *   (This is only called when the time slice ends with time remaining in the
 *   current CPU burst.  If finishing the time slice happens at the same time
 *   that the process blocks / terminates,
 *   then sched_blocked() / sched_terminated() will be called instead).
 *
 * proc - the process whose time slice just ended
 *
 * Note: Time slice end events only occur if use_time_slice() is set to TRUE
 */
void sched_finished_time_slice(const struct process* proc) {
  assert(READY == proc->state);
  // printf("in sched_finished_time_slice\n");
  pop();
  push(proc);
  if (!isEmpty() && !hasOne())
  {
    const struct process *next_proc = first();
    context_switch(next_proc->pid);
  }
}


/* sched_blocked
 *   will be called when the currently running process blocks
 *   (e.g., if it starts an I/O operation that it needs to wait to finish
 *
 * proc - the process that just blocked
 */
void sched_blocked(const struct process* proc) {
  assert(BLOCKED == proc->state);

  // printf("in sched_blocked\n");
  pop();
  if (!isEmpty())
  {
    const struct process *next_proc = first();
    context_switch(next_proc->pid);
  }
}


/* sched_unblocked
 *   will be called when a blocked process unblocks
 *   (e.g., if its I/O operation finished)
 *
 * proc - the process that just unblocked
 */
void sched_unblocked(const struct process* proc) {
  // printf("in sched_unblocked\n");
  assert(READY == proc->state);
  push(proc);
  if (hasOne())
  {
    context_switch(proc->pid);
  }
}


/* sched_terminated
 *   will be called when the currently running process terminates
 *   (i.e., it finished it's last CPU burst)
 *
 * proc - the process that just terminated
 *
 * Note: "kill" commands and other ways to terminate a process that is not
 *       currently running are not being simulated, so only the currently running
 *       process can actually terminate.
 */
void sched_terminated(const struct process* proc) {
  // printf("in sched_terminated\n");
  assert(TERMINATED == proc->state);
  pop();
  if (!isEmpty())
  {
    const struct process *next_proc = first();
    context_switch(next_proc->pid);
  }
}


/* sched_cleanup
 *   will be called exactly once after all processes have terminated and there
 *   are no more events left to occur, just before the simulation exits
 *
 * Note: Calling sched_cleanup() is guaranteed if the simulation has a normal exit
 *       but is not guaranteed in the case of fatal errors, crashes, or other
 *       abnormal exits.
 */
void sched_cleanup() {
  freeQueue(); 
}
