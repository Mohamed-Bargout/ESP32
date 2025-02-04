/*
  Real-time Concepts for Embedded Systems - Übung  - Assignment 3
  Team Members
  Aashik Muringathery Shoby (3762691)
  Allen Saldanha (3764343)
  Mohamed Bargout (3763454)
  Rico Haas (3310344)  
  */

#include <stdio.h>
#include "edf.h"
#include <math.h>

/* Iterative system density test for EDF. For 'task_id', it determines
 * whether all previously accepted tasks of params[0..task_id] remain
 * schedulable if we add the task params[task_id] as well.
 * The result (accept/reject + system density) is stored in results[task_id]. */
void system_density_test(PeriodicTaskParams **params, TaskId task_id,
                         AcceptanceTestResult *results) {
  double system_density = 0;
  bool accepted = false;

  for (TaskId i = 0; i <= task_id; i++) {
    /* Subtask 1: Complete the system density test for EDF
     * That is, you have to implement the following functionality:
     *  - Increment the system density
     *  - Set the variable 'accepted' accordingly.
     *  - Hint: Do not add previously rejected tasks to the system density!
     *    To get this information, use 'results[i]->accepted' for i < task_id.
     */
    if(results[i].accepted){
      if(params[i]->period<params[i]->deadline){
        system_density+=params[i]->execution_time/(double)params[i]->period;
      }
      else{
        system_density+=params[i]->execution_time/(double)params[i]->deadline;
      }
    }
  }
  // TODO: adjust 'accepted' according to the system density.
  // For now, this is set to accept T1-T3 to get the same schedule as in
  // the theoretical task 1b). I wonder what happens when you leave
  // it running like this for, let's say, 70 seconds?
  

  if(params[task_id]->period<params[task_id]->deadline){
    system_density+=params[task_id]->execution_time/(double)params[task_id]->period;
  }
  else{
    system_density+=params[task_id]->execution_time/(double)params[task_id]->deadline;
  }
  accepted = system_density <= 1 ? true : false;

  printf("Task %d: density %f, accepted %s\n", params[task_id]->id,
         system_density, accepted ? "true" : "false");

  results[task_id].system_density = system_density;
  results[task_id].accepted = accepted;
}

/* No need to change anything below this point... */

/* Determine the next job to execute under EDF.
 * Unfortunately, this EDF implementation is rather 'hacky' to
 * avoid having to check for overruns (something that is a must-have
 * in practice, of course). */
EDFInfo edf_select_next_job(PeriodicTaskParams **params,
                            TickType_t current_time_sec) {
  EDFInfo next_job = {NUMBER_OF_TASKS, ULONG_MAX, ULONG_MAX, false};
  for (TaskId i = 0; i < NUMBER_OF_TASKS; i++) {
    // skip deleted tasks
    if (params[i]->handle == NULL)
      continue;
    vTaskPrioritySet(params[i]->handle, taskIDLE_PRIORITY);

    eTaskState state = eTaskGetState(params[i]->handle);
    if (state == eReady) {
      // When the task is ready, determine next deadline.
      TickType_t deadline =
          params[i]->deadline - (current_time_sec % params[i]->period);
      printf(" State of Task%d: (deadline %lu, ready)\n", params[i]->id,
             deadline);
      if (params[i]->elapsed_time < params[i]->execution_time &&
          deadline < next_job.earliest_deadline) {
        next_job.earliest_deadline = deadline;
        next_job.next_task_id = i;
        if (params[i]->execution_time < next_job.next_scheduler_wakeup) {
          next_job.next_scheduler_wakeup =
              params[i]->execution_time - params[i]->elapsed_time;
          next_job.run_to_completion = true;
        }
      }
    } else if (state == eBlocked) {
      // When the task is blocked, determine next release time.
      // This is used to schedule the next wakeup time of the EDF scheduler.
      TickType_t next_release =
          params[i]->period - (current_time_sec % params[i]->period);
      printf(" State of Task%d: (next release %lu, blocked)\n", params[i]->id,
             next_release);
      if (next_release < next_job.next_scheduler_wakeup) {
        next_job.next_scheduler_wakeup = next_release;
        next_job.run_to_completion = false;
      }
    }
  }

  return next_job;
}

void edf_scheduler(PeriodicTaskParams **params) {
  printf("State Information: Running %d, Ready %d, Blocked %d, Suspended %d, "
         "Deleted %d\n",
         eRunning, eReady, eBlocked, eSuspended, eDeleted);

  vTaskPrioritySet(NULL, schedulerPRIORITY);
  TickType_t current_time_sec = 0;
  while (true) {
    printf("Time %lu\n", current_time_sec);

    // Check if there are new jobs to be released
    for (TaskId i = 0; i < NUMBER_OF_TASKS; i++) {
      if (params[i]->handle != NULL &&
          current_time_sec % params[i]->period == 0) {
        printf(" Release Task %d\n", params[i]->id);
        if (params[i]->elapsed_time != 0) {
          printf("OVERRUN DETECTED: Task %d did not reset correctly!\n",
                 params[i]->id);
          abort();
        }
        vTaskNotifyGiveFromISR(params[i]->handle, NULL);
      }
    }

    // Schedule job with the earliest deadline
    EDFInfo info = edf_select_next_job(params, current_time_sec);
    if (info.next_task_id < NUMBER_OF_TASKS) {
      vTaskPrioritySet(params[info.next_task_id]->handle, taskRUNNING_PRIORITY);
      printf(" Schedule %d: for %lu seconds\n", params[info.next_task_id]->id,
             info.next_scheduler_wakeup);

      // "Hacky" solution to synchronize EDF and the tasks. Otherwise, the
      // EDF scheduler typically wakes up before the tasks finishing time
      // and would have to terminate the unfinished task...
      for (TickType_t i = 0; i < info.next_scheduler_wakeup; i++) {
        ulTaskNotifyTake(false, portMAX_DELAY);
      }
      current_time_sec += info.next_scheduler_wakeup;
    } else {
      vTaskDelay(TICKS_PER_SECOND);
      current_time_sec++;
    }
  }
}

void edf_setup(unsigned int number_of_tasks) {
  NUMBER_OF_TASKS = number_of_tasks;
}
