#include "display.h"
#include "tasks.h"
#include <math.h>
#include <stdio.h>
#include <unistd.h>

/* No need to change anything here... */

#define TICKING_TASK_PRIORITY (tskIDLE_PRIORITY + 10)
#define TASK1_PRIORITY (tskIDLE_PRIORITY + 5)
#define TASK2_PRIORITY (tskIDLE_PRIORITY + 3)
#define TASK3_PRIORITY (tskIDLE_PRIORITY + 1)

CriticalSectionSemaphore cs1_semaphore;
CriticalSection task1_cs1 = {.resource = 0, .start = 2, .end = 3};
CriticalSection task3_cs1 = {.resource = 0, .start = 2, .end = 5};

PeriodicTaskParams task1_params = {.id = "T1",
                                   .release_time = 3,
                                   .execution_time = 4,
                                   .priority = TASK1_PRIORITY,
                                   .critical_sections = &task1_cs1,
                                   .no_of_critical_sections = 1,
                                   .gpio = mainTASK_TASK1_GPIO};

PeriodicTaskParams task2_params = {.id = "T2",
                                   .priority = TASK2_PRIORITY,
                                   .release_time = 6,
                                   .execution_time = 4,
                                   .critical_sections = NULL,
                                   .no_of_critical_sections = 0,
                                   .gpio = mainTASK_TASK2_GPIO};

PeriodicTaskParams task3_params = {.id = "T3",
                                   .priority = TASK3_PRIORITY,
                                   .release_time = 0,
                                   .execution_time = 6,
                                   .critical_sections = &task3_cs1,
                                   .no_of_critical_sections = 1,
                                   .gpio = mainTASK_TASK3_GPIO};

TickingTaskParams ticking_task_params = {
    .number_of_tasks = 3,
    .task_params = {&task1_params, &task2_params, &task3_params}};

void app_main(void) {
  task_setup();
  ssd1306_setup();

  PeriodicTaskParams *cs1_tasks[2] = {&task1_params, &task3_params};
  cs1_semaphore = usPrioritySemaphoreInit((void **)cs1_tasks, 2);
  task1_cs1.semaphore = &cs1_semaphore;
  task3_cs1.semaphore = &cs1_semaphore;

  // task initialization
  xTaskCreate((void *)periodic_task_implementation, "task1",
              configMINIMAL_STACK_SIZE + 256, &task1_params, TASK1_PRIORITY,
              &task1_params.handle);
  xTaskCreate((void *)periodic_task_implementation, "task2",
              configMINIMAL_STACK_SIZE + 256, &task2_params, TASK2_PRIORITY,
              &task2_params.handle);
  xTaskCreate((void *)periodic_task_implementation, "task3",
              configMINIMAL_STACK_SIZE + 256, &task3_params, TASK3_PRIORITY,
              &task3_params.handle);
  xTaskCreate((void *)ticking_task_implementation, "ticking_task",
              configMINIMAL_STACK_SIZE + 256, &ticking_task_params,
              TICKING_TASK_PRIORITY, NULL);
}
