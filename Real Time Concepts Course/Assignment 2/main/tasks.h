#ifndef TDA_TASK_H
#define TDA_TASK_H

/* No need to change anything here... */

#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>

#define mainTASK_OUTPUT_FREQUENCY_MS pdMS_TO_TICKS(1000UL)
#define mainBLINK_PER_TICK pdMS_TO_TICKS(50UL)

#define mainTASK_TASK1_GPIO GPIO_NUM_16
#define mainTASK_TASK2_GPIO GPIO_NUM_17
#define mainTASK_TASK3_GPIO GPIO_NUM_18
#define GPIO_PIN_BIT_MASK                                                      \
  ((1ULL << mainTASK_TASK1_GPIO) | (1ULL << mainTASK_TASK2_GPIO) |             \
   (1ULL << mainTASK_TASK3_GPIO))

typedef struct {
  char id;
  TickType_t execution_time;
  TickType_t period;
  TickType_t deadline;
  gpio_num_t gpio;
  TickType_t elapsed_time;
} TaskParams;

void task_useless_load(TaskParams *params, TickType_t duration);
void task_implementation(void *v_params);
void task_setup();

#endif
