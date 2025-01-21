#include "display.h"
#include "tasks.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>

typedef struct {
  bool accepted;
  TaskInfo task_info;
} AcceptanceTestResult;
AcceptanceTestResult default_result = {false, default_task_info};

TickType_t div_ceil(TickType_t x, TickType_t y) { return x / y + (x % y != 0); }

void utilization_bound_test(TaskParams **params, unsigned int task_id,
                            AcceptanceTestResult *result) {
    double utilization = 0;
    bool accepted = false;  

    /*--------------------------------------------------------------------
     * Subtask 1: Implement the Utilization Bound Test from the lecture
     * The goal is to determine if all tasks up to and including the task
     * with ID `task_id` are schedulable under Rate Monotonic Scheduling.
     ---------------------------------------------------------------------*/

    /* --- START~Solution --- */
    for (unsigned int i = 0; i <= task_id; ++i) {
        // Calculate the cumulative utilization of all tasks up to task_id.
        // Utilization of a task: execution_time / period.
        utilization += (double)params[i]->execution_time / params[i]->period;
    }

    // Calculate the utilization bound for n tasks:
    // n = number of tasks, which is task_id + 1 (since task_id is zero-based).
    unsigned int n = task_id + 1;
    double delta = (double)params[task_id]->deadline / params[task_id]->period; // Calculate Delta where deadline = Delta . period
    double util_max;

    // Use the correct equation according to the relative deadline
    if(delta <= 0.5){
      util_max = delta;
    }
    else if(delta <= 1){
      // util_max = n * (pow(2, 1.0 / n) - 1); // IF deadline = period (delta = 1)
      util_max = n * ( pow(2*delta, 1.0 / n) - 1) + 1 - delta;
    }
    else{
      util_max = delta * (n) * ( pow( (delta + 1)/delta, 1.0/(n) ) - 1); // n is used instead of n-1 to get the same numbers as in the table in the lecture
    }

    // Determine if the task set is schedulable.
    // If cumulative utilization <= maximum allowable utilization, accept.
    if(utilization <= util_max){
      accepted = true;
    }

    // Debugging : Print the calculated utilization and the bound.
    // printf("Utilization = %.3f\n", utilization);
    // printf("Maximum Allowable Utilization = %.3f\n", util_max);
    /* --- END~Solution --- */

    // Store the results in the result structure.
    result->accepted = accepted;
    result->task_info.util = utilization;
}

void worst_case_simulation(TaskParams **params, unsigned int task_id,
                           AcceptanceTestResult *result) {
    TickType_t completion_time = 0;
    bool accepted = false;

    /*--------------------------------------------------------------------
     * Subtask 2: Implement the Worst Case Simulation from the lecture
     * The goal is to calculate the worst-case completion time of the 
     * task under consideration (task_id) and check if it meets its deadline.
     ---------------------------------------------------------------------*/

    /* --- START~Solution --- */
    double time = 0;  // Temporary variable for calculating completion time as a double.

    // Sum up the contribution of each task of higher priority up to task under consideration to calculate the worst-case response time.
    for (int i = 0; i <= task_id; ++i) {
        // Calculate the Worst Case execution time according to formula
        // Iterating over all higher priority tasks as well as current Task.
        time += ceil( (double) params[task_id]->period / params[i]->period ) * params[i]->execution_time; // type cast is needed to avoid integer division which would result in floor instead of a ceil
    }

    // Round up the calculated time to the nearest integer using ceil, 
    // since tasks execute at integer time points the task will have to wait until the next integer time to run.
    completion_time = ceil(time);

    // Check if the calculated completion time is within the task's deadline (period).
    // If completion_time <= task's period, the task meets its deadline and is accepted.
    accepted = (completion_time <= params[task_id]->deadline) ? true : false; // Compare the worst case execution time to the period

    /* --- END~Solution --- */

    result->accepted = accepted;
    result->task_info.wcs_result = completion_time;
}


void time_demand_analysis(TaskParams **params, unsigned int task_id,
                          AcceptanceTestResult *result) {
    TickType_t t_last = 0, t_next = 0; 
    bool accepted = false;

    /*--------------------------------------------------------------------
     * Subtask 3: Implement the Time Demand Analysis from the lecture
     * The goal is to check if task `task_id` can meet its deadline.
     ---------------------------------------------------------------------*/
    
    /* --- START~Solution --- */

    // printf("\nTASK: %d\n", task_id);  // DEBUGGING

    // Step 1: Calculate t0
    for (int i = 0; i <= task_id; ++i) {
        t_last += params[i]->execution_time;  // Sum up execution times of all tasks up to task under consideration
    }
    // printf("T0: %ld\n", t_last);  // DEBUGGING

    // Step 2: Iterate to calculate the time demand until the system converges
    while (true) {
        // Calculate t_next (next demand time considering all tasks of higher priority up to task under consideration)
        t_next = 0;  // Reset t_next at the start of each iteration

        // Calculate t_next considering each task up to task_id
        for (int j = 0; j <= task_id; ++j) {
            t_next += ceil( (j == task_id) ? params[j]->execution_time : ceil((double)t_last / params[j]->period) * params[j]->execution_time );
            // ceil((double)t_last / params[j]->period) : ceil is used to perform the jump in the staircase for each multiple of higher priority task period
            // ceil is then applied to the entire value as t_next is of type TickType_t which does not accept floating point numbers, thus the value is ceiled to test worst case scenario
        }
        // printf("T_Next: %ld\n", t_next);  // DEBUGGING

        // Check for convergence or exceeding the deadline
        if (t_next > params[task_id]->deadline) {
            accepted = false;  // If t_next exceeds deadline, task is not schedulable
            break;
        }

        if (t_next == t_last) {
            accepted = true;  // If t_next equals t_last, the system has converged and since the value did not trigger the previous if condition to check for exceeding the deadline, it converged to a value less than the deadline
            break;
        }

        // Update t_last for the next iteration
        t_last = t_next;
    }

    /* --- END~Solution --- */

    // Store the results
    result->accepted = accepted; 
    result->task_info.tda_result = t_next;

    
}


/* Determine if params[task_id] can be scheduled.
 * - params: array of all task parameters (e.g., needed to perform TDA)
 * - task_id: index such that params[task_id] is the task under consideration
 * - results: output parameter yielding the acceptance test result */
void acceptance_test(TaskParams **params, unsigned int task_id,
                     AcceptanceTestResult *result) {

    /*--------------------------------------------------------------------
     * Subtask 4: Call the above acceptance tests in a suitable order.
     *  In particular, recall which of these tests are necessary,
     *  sufficient, or both.
     *  Ensure that the final value of result->accepted is true if and
     *  only if the task encoded by params[task_id] can be scheduled.
     ---------------------------------------------------------------------*/

    // Start by running tests to calculate Util, WCS, TDA values of task for display
    utilization_bound_test(params, task_id, result);
    worst_case_simulation(params, task_id, result);
    time_demand_analysis(params, task_id, result);

    // Start by assuming the task is schedulable.
    result->accepted = false;

    // Step 1: Run the Utilization Bound Test (Sufficient Condition)
    utilization_bound_test(params, task_id, result);
    if (result->accepted) {
        // If Utilization Bound succeeds, the task is schedulable
        return;  // No need to run further tests, the task is accepted.
    }

    // Step 2: Run the Worst-Case Simulation (Sufficient Condition)
    worst_case_simulation(params, task_id, result);
    if (result->accepted) {
        // If the Worst-Case Simulation succeeds, the task is schedulable.
        return;  // No need to run further tests, the task is accepted.
    }

    // Step 3: Run the Time Demand Analysis (Necessary & Sufficient Condition)
    time_demand_analysis(params, task_id, result);
    return; // Time demand is necessary and sufficient, therefore it is safe to continue with whatever result TDA comes to.

}


/* Tasks are scheduled according to RMA, i.e.,
 * prio(task1) > prio(task2) > prio(task3)
 * No need to change anything here... */
TaskParams task1_params = {.id = 1,
                           .execution_time = 2,
                           .period = 4,
                           .deadline = 4,
                           .gpio = mainTASK_TASK1_GPIO};
TaskParams task2_params = {.id = 2,
                           .execution_time = 2,
                           .period = 7,
                           .deadline = 7,
                           .gpio = mainTASK_TASK2_GPIO};
TaskParams task3_params = {.id = 3,
                           .execution_time = 1,
                           .period = 8,
                           .deadline = 8,
                           .gpio = mainTASK_TASK3_GPIO};

void app_main(void) {

  /*
  Real-time Concepts for Embedded Systems - Übung  - Assignment 2
  Team Members
  Aashik Muringathery Shoby (3762691)
  Allen Saldanha (3764343)
  Mohamed Bargout (3763454)
  Rico Haas (3310344)  
  */

  /* No need to change anything here... */
  task_setup();
  ssd1306_setup();

  TaskParams *task_set[3] = {&task1_params, &task2_params, &task3_params};
  AcceptanceTestResult results[3] = {default_result, default_result,
                                     default_result};

  /* --- START~DEBUGGING --- */

  // utilization_bound_test:

  // utilization_bound_test(task_set, 0, &results[0]); // Test for T1
  // printf("T1 Accepted: %s, Utilization: %.2f\n", results[0].accepted ? "Yes" : "No", results[0].task_info.util);

  // utilization_bound_test(task_set, 1, &results[1]); // Test for T2
  // printf("T2 Accepted: %s, Utilization: %.2f\n", results[1].accepted ? "Yes" : "No", results[1].task_info.util);

  // utilization_bound_test(task_set, 2, &results[2]); // Test for T3
  // printf("T3 Accepted: %s, Utilization: %.2f\n", results[2].accepted ? "Yes" : "No", results[2].task_info.util);



  // worst_case_simulation:

  // worst_case_simulation(task_set, 0, &results[0]); // Test for T1
  // printf("T1 Accepted: %s, Worse Case Time: %.d\n", results[0].accepted ? "Yes" : "No", (int)results[0].task_info.wcs_result);

  // worst_case_simulation(task_set, 1, &results[1]); // Test for T2
  // printf("T2 Accepted: %s, Worse Case Time: %.d\n", results[1].accepted ? "Yes" : "No", (int)results[1].task_info.wcs_result);

  // worst_case_simulation(task_set, 2, &results[2]); // Test for T3
  // printf("T3 Accepted: %s, Worse Case Time: %.d\n", results[2].accepted ? "Yes" : "No", (int)results[2].task_info.wcs_result);



  // time_demand_analysis:

  // time_demand_analysis(task_set, 0, &results[0]); // Test for T1
  // printf("T1 Accepted: %s, Time Demand Analaysis: %.ld\n", results[0].accepted ? "Yes" : "No", results[0].task_info.tda_result);

  // time_demand_analysis(task_set, 1, &results[1]); // Test for T2
  // printf("T2 Accepted: %s, Time Demand Analaysis: %.d\n", results[1].accepted ? "Yes" : "No", (int)results[1].task_info.tda_result);

  // time_demand_analysis(task_set, 2, &results[2]); // Test for T3
  // printf("T3 Accepted: %s, Time Demand Analaysis: %.d\n", results[2].accepted ? "Yes" : "No", (int)results[2].task_info.tda_result);



  // acceptance_test:

  // acceptance_test(task_set, 0, &results[0]); // Test for T1
  // printf("T1 Accepted: %s\n", results[0].accepted ? "Yes" : "No");

  // acceptance_test(task_set, 1, &results[1]); // Test for T2
  // printf("T2 Accepted: %s\n", results[1].accepted ? "Yes" : "No");

  // acceptance_test(task_set, 2, &results[2]); // Test for T3
  // printf("T3 Accepted: %s\n", results[2].accepted ? "Yes" : "No");

  /* --- END~DEBUGGING --- */
  

  // check acceptance test and store decision in results array
  for (unsigned int i = 0; i < 3; i++) {
    acceptance_test(task_set, i, &results[i]);
    if (!results[i].accepted)
      continue;

    // if accepted, create the task
    char task_name[6];
    snprintf(task_name, 6, "task%c", task_set[i]->id);
    xTaskCreate((void *)task_implementation, task_name,
                configMINIMAL_STACK_SIZE + 256, task_set[i],
                tskIDLE_PRIORITY + 3 - i, NULL);
  }

  // print acceptance test results
  for (;;) {
    for (unsigned int i = 0; i < 3; i++) {
      ssd1306_print_task_info(task_set[i], &results[i].task_info);
      sleep(1);
    }
  }


}
