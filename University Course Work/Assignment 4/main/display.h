#ifndef ICPP_SSD1306_H
#define ICPP_SSD1306_H

/* No need to change anything here... */

#include "critical_section.h"
#include "freertos/FreeRTOS.h"
#include "sdkconfig.h"
#include "ssd1306.h"
#include <stdio.h>

#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_1
#define I2C_MASTER_FREQ_HZ 100000

typedef struct {
  TickType_t tick;
  char *task_in_cs[RESOURCES];
} DisplayedState;

void ssd1306_setup();
void ssd1306_print(DisplayedState *state);

#endif
