#include "scheduler.h"
#include <assert.h>

/********************
 * STRIDE Scheduler *
 ********************/


#define STRIDE_CONSTANT 1000000


/* sched_init
 *   will be called exactly once before any processes arrive or any other events
 */
void sched_init() {
  use_time_slice(TRUE);
  // TODO: implement this
}


/* sched_new_process
 *   will be called when a new process arrives (i.e., fork())
 *
 * proc - the new process that just arrived
 */
void sched_new_process(const struct process* proc) {
  assert(READY == proc->state);
  // TODO: implement this
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
  // TODO: implement this
}


/* sched_blocked
 *   will be called when the currently running process blocks
 *   (e.g., if it starts an I/O operation that it needs to wait to finish
 *
 * proc - the process that just blocked
 */
void sched_blocked(const struct process* proc) {
  assert(BLOCKED == proc->state);
  // TODO: implement this
}


/* sched_unblocked
 *   will be called when a blocked process unblocks
 *   (e.g., if its I/O operation finished)
 *
 * proc - the process that just unblocked
 */
void sched_unblocked(const struct process* proc) {
  assert(READY == proc->state);
  // TODO: implement this
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
  assert(TERMINATED == proc->state);
  // TODO: implement this
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
  // TODO: implement this
}

