#include "tasks.h"
#include <unistd.h>

void periodic_server_implementation(void *v_params) {
  // cast needed as xTaskCreate expects void pointer in first arg
  PeriodicTaskParams *params = (PeriodicTaskParams *)v_params;

  for (;;) {
    // wait for the release signal from the EDF scheduler
    ulTaskNotifyTake(true, portMAX_DELAY);

    /* Subtask 2: Complete the periodic server
     * An aperiodic task is released whenever the button is pressed.
     * The number of ready aperiodic tasks is (for simplicity) stored
     * in the counter 'aperiodic_tasks_available'. Moreover, each
     * aperiodic task executes for one second (i.e., for the entire
     * execution time of the periodic server).
     *
     * In detail, implement the following functionality:
     *  - For each available aperiodic task, call
     *    'ssd1306_print_aperiodic_task()' to trigger a animation on
     *    the display. The duration of this animation is one second.
     *  - Update 'aperiodic_tasks_available' after finishing an
     *    aperiodic task.
     *  - Notify the schedulerTask if either:
     *    -> there are no remaining aperiodic tasks and the periodic
     *       server yields its execution time, the periodic server
     *       will still loose the budget
     *    -> the periodic server completed an aperiodic task
     *       (this is solely used for synchronization; otherwise, the
     *       EDF scheduler may not continue)
     *    To notify the schedulerTask, use
     *    'vTaskNotifyGiveFromISR(schedulerTask, NULL)'
     */

    // default behavior: yield execution right back to the EDF scheduler
    printf(" Button Pressed %ld Times\n", aperiodic_tasks_available);
    vTaskNotifyGiveFromISR(schedulerTask, NULL);

    if(aperiodic_tasks_available>0){
      while(aperiodic_tasks_available>0){
        ssd1306_print_aperiodic_task();
        aperiodic_tasks_available--;
        vTaskNotifyGiveFromISR(schedulerTask, NULL);
      }
    }
    else{
      vTaskNotifyGiveFromISR(schedulerTask, NULL);
    }
  }
}

/* No need to change anything below this point... */

void busy_waiting(TickType_t until) {
  while (xTaskGetTickCount() < until) {
  }
}

/* Useless load for 'duration' (in seconds) with busy waiting.
 * After each second, the schedulerTask is notified using
 * 'vTaskNotifyGiveFromISR'. This is essentially a counting
 * semaphore to synchronize the EDF scheduler and the tasks
 * (reducing the race-conditions and avoiding having to terminate
 * overrunning tasks). */
void task_useless_load(PeriodicTaskParams *params, TickType_t duration) {
  TickType_t i;
  unsigned long j;

  for (i = 0; i < duration; ++i) {
    TickType_t time_unit_start = xTaskGetTickCount();
    printf(" Execute: Task %d (%ld/%ld)\n", params->id,
           params->elapsed_time + 1, params->execution_time);
    // we use 800 instead of 1000 for a conservative WCET of 1 second
    for (j = 0; j < 800 / BLINKING_SLEEP_MS; ++j) {
      busy_waiting(time_unit_start +
                   (2 * j + 1) * pdMS_TO_TICKS(BLINKING_SLEEP_MS));
      gpio_set_level(params->gpio, 0);
      busy_waiting(time_unit_start +
                   2 * (j + 1) * pdMS_TO_TICKS(BLINKING_SLEEP_MS));
      gpio_set_level(params->gpio, 1);
    }
    params->elapsed_time++;
    vTaskNotifyGiveFromISR(schedulerTask, NULL);
  }
}

void periodic_task_implementation(void *v_params) {
  // cast needed as xTaskCreate expects void pointer in first arg
  PeriodicTaskParams *params = (PeriodicTaskParams *)v_params;

  // indicate that task is ready
  ulTaskNotifyTake(true, portMAX_DELAY);
  gpio_set_level(params->gpio, 1);

  params->elapsed_time = 0;
  for (;;) {
    task_useless_load(params, params->execution_time);
    gpio_set_level(params->gpio, 0);
    printf(" Complete: Task %d\n", params->id);

    params->elapsed_time = 0;
    ulTaskNotifyTake(true, portMAX_DELAY);
    gpio_set_level(params->gpio, 1);
  }
}

/* Interrupt service routine after button press.
 * Bonus Task: Make it thread-safe. */
static void IRAM_ATTR gpio_isr_handler(void *arg) {
  aperiodic_tasks_available++;
}

// Register GPIO pins for LEDs and for the button
void task_setup() {
  gpio_config_t io_conf_in = {.pin_bit_mask = GPIO_INPUT_PIN_SEL,
                              .mode = GPIO_MODE_INPUT,
                              .pull_up_en = 1,
                              .pull_down_en = 0,
                              .intr_type = GPIO_INTR_POSEDGE};
  gpio_config(&io_conf_in);
  gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);
  gpio_isr_handler_add(GPIO_INPUT_BTN, gpio_isr_handler,
                       (void *)GPIO_INPUT_BTN);

  gpio_config_t io_conf_out = {.pin_bit_mask = GPIO_PIN_BIT_MASK,
                               .mode = GPIO_MODE_OUTPUT,
                               .pull_up_en = 0,
                               .pull_down_en = 0,
                               .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf_out);
  schedulerTask = xTaskGetCurrentTaskHandle();
}
