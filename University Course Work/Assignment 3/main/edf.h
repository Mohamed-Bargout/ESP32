#ifndef EDF_H
#define EDF_H

#include "freertos/FreeRTOS.h"
#include "tasks.h"

/* No need to change anything here... */
typedef unsigned int TaskId;

#define taskIDLE_PRIORITY tskIDLE_PRIORITY
#define taskRUNNING_PRIORITY tskIDLE_PRIORITY + 1
#define schedulerPRIORITY tskIDLE_PRIORITY + 2

static unsigned int NUMBER_OF_TASKS;

typedef struct {
  bool accepted;
  double system_density;
} AcceptanceTestResult;
static const AcceptanceTestResult default_result = {false, 0};

typedef struct {
  TaskId next_task_id;
  TickType_t earliest_deadline;
  TickType_t next_scheduler_wakeup;
  bool run_to_completion;
} EDFInfo;

void system_density_test(PeriodicTaskParams **params, TaskId task_id,
                         AcceptanceTestResult *results);

EDFInfo edf_select_next_job(PeriodicTaskParams **params,
                            TickType_t current_time_sec);
void edf_scheduler(PeriodicTaskParams **params);
void edf_setup(unsigned int number_of_tasks);

#endif
