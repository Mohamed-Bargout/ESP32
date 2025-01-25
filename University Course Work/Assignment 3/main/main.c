#include "display.h"
#include "edf.h"
#include "tasks.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>

/* For this practical task, we provide you with a
 * custom implementation of an Earliest Deadline First scheduler.
 * It is your job to extend this implementation by a system density test
 * and a periodic server that runs aperiodic tasks. To summarize, you should:
 * 1. Complete 'system_density_test' that is located in edf.c.
 * 2. Complete 'periodic_server_implementation' that is located in tasks.c.
 * You'll find further instructions in the respective files.
 */

#define NUMBER_OF_TASKS 4

PeriodicTaskParams task1_params = {
    .id = 1,
    .execution_time = 2,
    .period = 5,
    .deadline = 5,
    .gpio = mainTASK_TASK1_GPIO,
    .type = PERIODIC_TASK,
};
PeriodicTaskParams task2_params = {
    .id = 2,
    .execution_time = 1,
    .period = 3,
    .deadline = 3,
    .gpio = mainTASK_TASK2_GPIO,
    .type = PERIODIC_TASK,
};
PeriodicTaskParams task3_params = {
    .id = 3,
    .execution_time = 2,
    .period = 7,
    .deadline = 7,
    .gpio = mainTASK_TASK3_GPIO,
    .type = PERIODIC_TASK,
};
PeriodicTaskParams ps_params = {
    .id = 4,
    .execution_time = 1,
    .period = 7,
    .deadline = 7,
    .gpio = No_GPIO,
    .type = PERIODIC_SERVER,
};

/* No need to change anything here... */
void app_main(void) {
  task_setup();
  ssd1306_setup();
  edf_setup(NUMBER_OF_TASKS);

  PeriodicTaskParams *task_set[NUMBER_OF_TASKS] = {&task1_params, &task2_params,
                                                   &task3_params, &ps_params};
  AcceptanceTestResult results[NUMBER_OF_TASKS];

  // Run system density test for all tasks
  for (TaskId i = 0; i < NUMBER_OF_TASKS; i++) {
    results[i] = default_result;
    system_density_test(task_set, i, results);
  }

  // Create tasks and provide the edf scheduler with the task handles
  for (TaskId i = 0; i < NUMBER_OF_TASKS; i++) {
    if (!results[i].accepted)
      continue;

    if (task_set[i]->type == PERIODIC_TASK) {
      char task_name[6];
      snprintf(task_name, 6, "task%c", task_set[i]->id);
      xTaskCreate((void *)periodic_task_implementation, task_name,
                  configMINIMAL_STACK_SIZE + 256, task_set[i], tskIDLE_PRIORITY,
                  &task_set[i]->handle);
    } else if (task_set[i]->type == PERIODIC_SERVER) {
      xTaskCreate((void *)periodic_server_implementation, "PS",
                  configMINIMAL_STACK_SIZE + 256, task_set[i], tskIDLE_PRIORITY,
                  &task_set[i]->handle);
    }
  }

  edf_scheduler(task_set);
}
