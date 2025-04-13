#include "scheduler.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

/*******************************************************
 * SHORTEST TIME TO COMPLETION getFirst (STCF) Scheduler  *
 * (also known as Shortest Remaining Time getFirst (SRTF) *
 *******************************************************/

typedef struct
{
  const struct process **array;
  int size;
  int capacity;
} newQueue;

static newQueue *queue = NULL;

newQueue *createQueue()
{
  newQueue *queue = (newQueue *)malloc(sizeof(newQueue));
  queue->capacity = 10;
  queue->size = 0;
  queue->array = (const struct process **)malloc(10 * sizeof(const struct process *));
  return queue;
}

void resizeQueue()
{
  queue->capacity *= 2;
  queue->array = (const struct process **)realloc(queue->array, queue->capacity * sizeof(const struct process *));
}

void push(const struct process *proc)
{
  if (queue->size == queue->capacity)
  {
    resizeQueue();
  }
  int i = queue->size - 1;
  while (i >= 0 && queue->array[i]->current_burst->remaining_time > proc->current_burst->remaining_time)
  {
    queue->array[i + 1] = queue->array[i];
    i--;
  }
  queue->array[i + 1] = proc;
  queue->size++;
}

const struct process *getFirst()
{
  return queue->array[0];
}

const struct process *pop()
{
  if (queue->size == 0)
    return NULL;
  const struct process *proc = queue->array[0];
  for (int i = 0; i < queue->size - 1; i++)
  {
    queue->array[i] = queue->array[i + 1];
  }
  queue->size--;
  return proc;
}

void removeFromQueue(const struct process *proc)
{
  int i, found = 0;
  for (i = 0; i < queue->size; i++)
  {
    if (queue->array[i] == proc)
    {
      found = 1;
      break;
    }
  }
  if (found)
  {
    for (; i < queue->size - 1; i++)
    {
      queue->array[i] = queue->array[i + 1];
    }
    queue->size--;
  }
}

int hasOne()
{
  return queue->size == 1;
}

int isEmpty()
{
  return queue->size == 0;
}

void removeFromQueueProcess(const struct process *proc)
{
  int i, found = 0;
  for (i = 0; i < queue->size; i++)
  {
    if (queue->array[i] == proc)
    {
      found = 1;
      break;
    }
  }
  if (found)
  {
    for (; i < queue->size - 1; i++)
    {
      queue->array[i] = queue->array[i + 1];
    }
    queue->size--;
  }
}

void freeQueue()
{
  free(queue->array);
  free(queue);
}

/* sched_init
 *   will be called exactly once before any processes arrive or any other events
 */
void sched_init()
{
  use_time_slice(FALSE);
  queue = createQueue();
}

/* sched_new_process
 *   will be called when a new process arrives (i.e., fork())
 *
 * proc - the new process that just arrived
 */
void sched_new_process(const struct process *proc)
{
  printf("     sched_new_process\n");
  assert(READY == proc->state);
  push(proc);
  const struct process *nextProc = getFirst();

  if (!hasOne() && nextProc->pid == proc->pid)
  {
    context_switch(nextProc->pid);
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
void sched_finished_time_slice(const struct process *proc)
{
  printf("     sched_finished_time_slice\n");
  assert(READY == proc->state);
  removeFromQueue(proc);
  push(proc);
  if (hasOne())
  {
    context_switch(queue->array[0]->pid);
  }
  else
  {
    const struct process *nextProc = getFirst();
    context_switch(nextProc->pid);
  }
}

/* sched_blocked
 *   will be called when the currently running process blocks
 *   (e.g., if it starts an I/O operation that it needs to wait to finish
 *
 * proc - the process that just blocked
 */
void sched_blocked(const struct process *proc)
{
  printf("     sched_blocked\n");
  assert(BLOCKED == proc->state);
  removeFromQueue(proc);
  if (!isEmpty())
  {
    context_switch(queue->array[0]->pid);
  }
}

/* sched_unblocked
 *   will be called when a blocked process unblocks
 *   (e.g., if its I/O operation finished)
 *
 * proc - the process that just unblocked
 */
void sched_unblocked(const struct process *proc)
{
  printf("     sched_unblocked\n");
  assert(READY == proc->state);
  push(proc);
  if (hasOne())
  {
    context_switch(proc->pid);
  }
  else
  {
    const struct process *nextProc = getFirst();
    context_switch(nextProc->pid);
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
void sched_terminated(const struct process *proc)
{
  printf("     sched_terminated\n");
  assert(TERMINATED == proc->state);
  removeFromQueue(proc);
  if (hasOne())
  {
    context_switch(queue->array[0]->pid);
  }
  else
  {
    const struct process *nextProc = getFirst();
    context_switch(nextProc->pid);
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
void sched_cleanup()
{
  freeQueue();
}
