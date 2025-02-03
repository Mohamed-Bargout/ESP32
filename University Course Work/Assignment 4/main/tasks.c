#include "tasks.h"
#include <unistd.h>

DisplayedState state;
SemaphoreHandle_t button_semaphore, tick_semaphore, blinking_semaphore;
TickType_t last_button_press = 0;

void busy_waiting(TickType_t until) {
  while (xTaskGetTickCount() < until) {
  }
}

void task_useless_load(PeriodicTaskParams *params, TickType_t duration) {
  TickType_t i;

  for (i = 0; i < duration; ++i) {
    xSemaphoreTake(tick_semaphore, portMAX_DELAY);
    printf("EXEC: Task %s (%lu/%lu)\n", params->id, params->elapsed_time + 1,
           params->execution_time);
    // update display information
    ssd1306_print(&state);

    // continue blinking until button is pressed again
    int current_tick = state.tick;
    while (current_tick == state.tick) {
      TickType_t time_unit_start = xTaskGetTickCount();
      busy_waiting(time_unit_start + pdMS_TO_TICKS(BLINKING_SLEEP_MS));
      gpio_set_level(params->gpio, 0);
      busy_waiting(time_unit_start + 2 * pdMS_TO_TICKS(BLINKING_SLEEP_MS));
      gpio_set_level(params->gpio, 1);
    }
    params->elapsed_time++;
  }
}

void periodic_task_implementation(void *v_params) {
  PeriodicTaskParams *params = (PeriodicTaskParams *)v_params;

  // initially, suspend task until it is released by ticking_task
  vTaskSuspend(NULL);

  // trigger LED, indicating that task is ready
  gpio_set_level(params->gpio, 1);
  printf("RELEASE: Task %s\n", params->id);
  params->elapsed_time = 0;

  // task execution, including critical sections
  int i;
  for (i = 0; i < params->no_of_critical_sections; ++i) {
    CriticalSection cs = params->critical_sections[i];
    // execution until reaching critical section
    task_useless_load(params, cs.start - params->elapsed_time);

    // critical section
    printf("CS (AWAIT): Task %s (%d/%u)\n", params->id, i + 1,
           params->no_of_critical_sections);
    usPrioritySemaphoreWait(cs.semaphore);
    params->priority = uxTaskPriorityGet(NULL);
    printf("CS (ENTER): Task %s (%d/%u)\n", params->id, i + 1,
           params->no_of_critical_sections);
    state.task_in_cs[cs.resource] = params->id;
    task_useless_load(params, cs.end - cs.start);
    state.task_in_cs[cs.resource] = NULL;
    printf("CS (LEAVE): Task %s (%d/%u)\n", params->id, i + 1,
           params->no_of_critical_sections);
    usPrioritySemaphoreSignal(cs.semaphore);
    params->priority = uxTaskPriorityGet(NULL);
  }
  task_useless_load(params, params->execution_time - params->elapsed_time);
  gpio_set_level(params->gpio, 0);

  printf("COMPLETE: Task %s\n", params->id);

  // delete task instance
  vTaskDelete(NULL);
}

void ticking_task_implementation(void *vparams) {
  TickingTaskParams *params = (TickingTaskParams *)vparams;
  int i;

  while (1) {
    // button press initiates next tick
    xSemaphoreTake(button_semaphore, portMAX_DELAY);
    for (i = 0; i < params->number_of_tasks; i++) {
      if (state.tick == params->task_params[i]->release_time) {
        vTaskResume(params->task_params[i]->handle);
      }
    }
    state.tick++;
    // wait for current blinking routine to finish
    xSemaphoreTake(blinking_semaphore, portMAX_DELAY);
    xSemaphoreGive(tick_semaphore);
    xSemaphoreGive(blinking_semaphore);
  }
}

void IRAM_ATTR gpio_isr_handler(void *arg) {
  TickType_t time = xTaskGetTickCount();
  // avoid registering button press multiple times
  if (time - last_button_press > TICKS_PER_SECOND) {
    xSemaphoreGive(button_semaphore);
    last_button_press = time;
  }
}

void task_setup() {
  // GPIO setup to increase tick when button is pressed
  gpio_config_t io_conf_in = {.pin_bit_mask = GPIO_INPUT_PIN_SEL,
                              .mode = GPIO_MODE_INPUT,
                              .pull_up_en = 1,
                              .pull_down_en = 0,
                              .intr_type = GPIO_INTR_POSEDGE};
  gpio_config(&io_conf_in);
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(GPIO_INPUT_BTN, gpio_isr_handler,
                       (void *)GPIO_INPUT_BTN);

  // GPIO setup of LEDs
  gpio_config_t io_conf_out = {.pin_bit_mask = GPIO_PIN_BIT_MASK,
                               .mode = GPIO_MODE_OUTPUT,
                               .pull_up_en = 0,
                               .pull_down_en = 0,
                               .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf_out);

  tick_semaphore = xSemaphoreCreateBinary();
  blinking_semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(blinking_semaphore);
  button_semaphore = xSemaphoreCreateBinary();
}
