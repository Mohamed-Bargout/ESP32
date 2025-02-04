#ifndef TDA_SSD1306_H
#define TDA_SSD1306_H

/* No need to change anything here... */

#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "ssd1306.h"
#include "tasks.h"
#include <stdio.h>

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_1
#define I2C_MASTER_FREQ_HZ 100000

typedef struct {
  double util;
  TickType_t wcs_result;
  TickType_t tda_result;
} TaskInfo;

static const TaskInfo default_task_info = {0, 0, 0};

void ssd1306_setup();
void ssd1306_print_task_info(TaskParams *params, TaskInfo *task_info);

#endif
