#include "display.h"

ssd1306_handle_t ssd1306_dev = NULL;
void ssd1306_setup() {
  i2c_config_t conf;
  conf.mode = I2C_MODE_MASTER;
  conf.sda_io_num = (gpio_num_t)I2C_MASTER_SDA_IO;
  conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
  conf.scl_io_num = (gpio_num_t)I2C_MASTER_SCL_IO;
  conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
  conf.master.clk_speed = I2C_MASTER_FREQ_HZ;
  conf.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL;

  i2c_param_config(I2C_MASTER_NUM, &conf);
  i2c_driver_install(I2C_MASTER_NUM, conf.mode, 0, 0, 0);

  // deprecated, you can get bonus points for updating it ;-)
  ssd1306_dev = ssd1306_create(I2C_MASTER_NUM, SSD1306_I2C_ADDRESS);
  ssd1306_refresh_gram(ssd1306_dev);
  ssd1306_clear_screen(ssd1306_dev, 0x00);
}

void ssd1306_print_task_info(TaskParams *params, TaskInfo *task_info) {
  char val[20];
  ssd1306_clear_screen(ssd1306_dev, 0x00);

  // print task name
  snprintf(val, 20, "Task %d: (%lu,%lu)", params->id, params->period,
           params->execution_time);
  ssd1306_draw_string(ssd1306_dev, 10, 0, (const uint8_t *)val, 16, 1);

  // print cpu utilization
  ssd1306_draw_string(ssd1306_dev, 10, 16, (const uint8_t *)"Util:", 16, 1);
  if (task_info->util > 0) {
    snprintf(val, 16, "%.03f", task_info->util);
    ssd1306_draw_string(ssd1306_dev, 70, 16, (const uint8_t *)val, 16, 1);
  } else {
    ssd1306_draw_string(ssd1306_dev, 70, 16, (const uint8_t *)"-", 16, 1);
  }

  // print completion time test
  ssd1306_draw_string(ssd1306_dev, 10, 32, (const uint8_t *)"WCS:", 16, 1);
  if (task_info->wcs_result > 0) {
    snprintf(val, 16, "%lu", task_info->wcs_result);
    ssd1306_draw_string(ssd1306_dev, 70, 32, (const uint8_t *)val, 16, 1);
  } else {
    ssd1306_draw_string(ssd1306_dev, 70, 32, (const uint8_t *)"-", 16, 1);
  }

  // print completion time test
  ssd1306_draw_string(ssd1306_dev, 10, 48, (const uint8_t *)"TDA:", 16, 1);
  if (task_info->tda_result > 0) {
    snprintf(val, 16, "%lu", task_info->tda_result);
    ssd1306_draw_string(ssd1306_dev, 70, 48, (const uint8_t *)val, 16, 1);
  } else {
    ssd1306_draw_string(ssd1306_dev, 70, 48, (const uint8_t *)"-", 16, 1);
  }

  ssd1306_refresh_gram(ssd1306_dev);
}
