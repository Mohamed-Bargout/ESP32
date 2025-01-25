#ifndef EDF_TASK_H
#define EDF_TASK_H

/* No need to change anything here... */

#include "display.h"
#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>

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

enum TaskType_t { PERIODIC_TASK, PERIODIC_SERVER };
typedef enum TaskType_t TaskType_t;

static TaskHandle_t schedulerTask = NULL;
static uint32_t aperiodic_tasks_available = 0;

typedef struct {
  char id;
  TickType_t execution_time;
  TickType_t period;
  TickType_t deadline;
  gpio_num_t gpio;
  TaskType_t type;
  TickType_t elapsed_time;
  TaskHandle_t handle;
} PeriodicTaskParams;

void task_useless_load(PeriodicTaskParams *params, TickType_t duration);
void periodic_task_implementation(void *v_params);
void periodic_server_implementation(void *v_params);
void task_setup();

#endif
