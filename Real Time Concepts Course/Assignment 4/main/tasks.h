#ifndef ICPP_TASK_H
#define ICPP_TASK_H

/* No need to change anything here... */

#include "critical_section.h"
#include "display.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <inttypes.h>

#define TICKS_PER_SECOND pdMS_TO_TICKS(1000UL)
#define BLINKING_SLEEP_MS 100

#define mainTASK_TASK1_GPIO GPIO_NUM_16
#define mainTASK_TASK2_GPIO GPIO_NUM_17
#define mainTASK_TASK3_GPIO GPIO_NUM_18

#define No_GPIO 0
#define GPIO_PIN_BIT_MASK                                                      \
  ((1ULL << mainTASK_TASK1_GPIO) | (1ULL << mainTASK_TASK2_GPIO) |             \
   (1ULL << mainTASK_TASK3_GPIO))

#define ESP_INTR_FLAG_DEFAULT 0
#define GPIO_INPUT_BTN GPIO_NUM_15
#define GPIO_INPUT_PIN_SEL 1ULL << GPIO_INPUT_BTN

typedef struct {
  char *id;
  TickType_t release_time;
  TickType_t execution_time;
  PriorityType_t priority;
  CriticalSection *critical_sections;
  size_t no_of_critical_sections;
  gpio_num_t gpio;
  TickType_t elapsed_time;
  TaskHandle_t handle;
} PeriodicTaskParams;

typedef struct {
  int number_of_tasks;
  PeriodicTaskParams *task_params[];
} TickingTaskParams;

void task_useless_load(PeriodicTaskParams *params, TickType_t duration);
void periodic_task_implementation(void *v_params);
void ticking_task_implementation(void *vparams);
void task_setup();

#endif
