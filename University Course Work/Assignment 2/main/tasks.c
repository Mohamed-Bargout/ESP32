#include "tasks.h"

/* No need to change anything here... */

SemaphoreHandle_t useless_load_semaphore;
void task_useless_load(TaskParams *params, TickType_t duration) {
  TickType_t i;
  unsigned long j;

  for (i = 0; i < duration; ++i) {
    xSemaphoreTake(useless_load_semaphore, portMAX_DELAY);
    TickType_t next_wake_time = xTaskGetTickCount();
    printf("EXEC: Task %d (%ld/%ld)\n", params->id, params->elapsed_time + 1,
           params->execution_time);
    for (j = 0; j < mainBLINK_PER_TICK; ++j) {
      vTaskDelayUntil(&next_wake_time,
                      mainTASK_OUTPUT_FREQUENCY_MS / (2 * mainBLINK_PER_TICK));
      gpio_set_level(params->gpio, 0);
      vTaskDelayUntil(&next_wake_time,
                      mainTASK_OUTPUT_FREQUENCY_MS / (2 * mainBLINK_PER_TICK));
      gpio_set_level(params->gpio, 1);
    }
    params->elapsed_time++;
    xSemaphoreGive(useless_load_semaphore);
  }
}

void task_implementation(void *v_params) {
  // cast needed as xTaskCreate expects void pointer in first arg
  TaskParams *params = (TaskParams *)v_params;

  // indicate that task is ready
  printf("RELEASE: Task %d\n", params->id);
  gpio_set_level(params->gpio, 1);
  TickType_t next_wake_time = xTaskGetTickCount();
  const TickType_t period = params->period * mainTASK_OUTPUT_FREQUENCY_MS;

  for (;;) {
    params->elapsed_time = 0;
    task_useless_load(params, params->execution_time);
    gpio_set_level(params->gpio, 0);
    printf("COMPLETE: Task %d\n", params->id);

    vTaskDelayUntil(&next_wake_time, period);
    printf("RELEASE: Task %d\n", params->id);
    gpio_set_level(params->gpio, 1);
  }
}

void task_setup() {
  gpio_config_t io_conf = {.pin_bit_mask = GPIO_PIN_BIT_MASK,
                           .mode = GPIO_MODE_OUTPUT,
                           .pull_up_en = 0,
                           .pull_down_en = 0,
                           .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);

  // semaphore to ensure tasks are only preemted after a timeslot
  useless_load_semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(useless_load_semaphore);
}
