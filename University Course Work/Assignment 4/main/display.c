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

void ssd1306_print(DisplayedState *state) {
  char val[20];
  ssd1306_clear_screen(ssd1306_dev, 0x00);
  // print tick
  ssd1306_draw_string(ssd1306_dev, 10, 0, (const uint8_t *)"Tick", 16, 1);
  snprintf(val, 10, "%lu", state->tick - 1);
  ssd1306_draw_string(ssd1306_dev, 70, 0, (const uint8_t *)val, 16, 1);
  // print running task
  ssd1306_draw_string(ssd1306_dev, 10, 16, (const uint8_t *)"Tasks in CS:", 16,
                      1);

  for (int r = 0; r < RESOURCES; r++) {
    if (state->task_in_cs[r] != NULL) {
      snprintf(val, 20, "R%d: %s", r, state->task_in_cs[r]);
      ssd1306_draw_string(ssd1306_dev, 10 + r * 50, 32, (const uint8_t *)val,
                          16, 1);
    }
  }

  ssd1306_refresh_gram(ssd1306_dev);
}
