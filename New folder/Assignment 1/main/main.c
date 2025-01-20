/*
  Real-time Concepts for Embedded Systems - Übung  - Assignment 1
  Team Members
  Aashik Muringathery Shoby (3762691)
  Allen Saldanha (3764343)
  Mohamed Bargout (3763454)
  Rico Haas (3310344)  
  */

#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include <inttypes.h>
#include <stdio.h>

// helpful to use seconds as the default time unit
#define mainTASK_CHATTERBOX_OUTPUT_FREQUENCY pdMS_TO_TICKS(1000UL)

// specifies LED blinking rate when tasks are executing
#define mainBLINK_PER_TICK pdMS_TO_TICKS(50UL)

// Constants to specify the GPIO assignment of each task
#define mainTASK_CHATTERBOX_TASK1_GPIO GPIO_NUM_16
#define mainTASK_CHATTERBOX_TASK2_GPIO GPIO_NUM_17
#define mainTASK_CHATTERBOX_TASK3_GPIO GPIO_NUM_18
#define GPIO_PIN_BIT_MASK                                                      \
  ((1ULL << mainTASK_CHATTERBOX_TASK1_GPIO) |                                  \
   (1ULL << mainTASK_CHATTERBOX_TASK2_GPIO) |                                  \
   (1ULL << mainTASK_CHATTERBOX_TASK3_GPIO))

/* TaskParams specifies the behavior of each task and is later passed to
 * chatterbox_task. The values of release_time, execution_time, and period are
 * given in seconds. The field repetitions provides the means to limit the
 * number of executions for a given task. gpio should use the above assignment
 * for its dedicated task. elapsed_time keeps track how many time units the task
 * is executed (initialize as 0).
 */
typedef struct {
  char id;
  TickType_t release_time;
  TickType_t execution_time;
  TickType_t period;
  unsigned int repetitions;
  gpio_num_t gpio;
  TickType_t elapsed_time;
} TaskParams;

/* Helper function to emulate some work for a given task.
 * During execution, the configured LED is blinking at the rate specified by
 * mainBLINK_PER_TICK.
 */
SemaphoreHandle_t useless_load_semaphore; //used to control access to shared resources
//useless_load is used to control the number of LED blinks
static void useless_load(TaskParams *params, TickType_t duration) {
  TickType_t i;
  unsigned long j;

  for (i = 0; i < duration; ++i) {
    //here "portMAX_DELAY" is used to ensure that the function will wait till as long as the  
    //semaphore is occupied, that is being used by another task
    xSemaphoreTake(useless_load_semaphore, portMAX_DELAY);

    //xTasgGetTickCount() gets the number of ticks since the system has started
    //we are storing it in next_wake_time to ensure the task's timings and precise delays
    TickType_t next_wake_time = xTaskGetTickCount();

    printf("EXEC: Task %d (%ld/%ld)\n", params->id, params->elapsed_time + 1,params->execution_time);

    for (j = 0; j < mainBLINK_PER_TICK; ++j) {//controls how many blinks happen per system tick 
      //vTaskDelayUntil() specifies the absolute (exact) time at which it wishes to unblock.
      //&next_wake_time is the Pointer to a variable that holds the time at which the task was last unblocked
      //the second parameter+&next_wake_time is when the task will be unblocked
      vTaskDelayUntil(&next_wake_time, mainTASK_CHATTERBOX_OUTPUT_FREQUENCY/(2 * mainBLINK_PER_TICK));
      //Sets the GPIO pin (specified by params->gpio) to a low voltage level (0), which typically turns the LED off.
      gpio_set_level(params->gpio, 0);
      vTaskDelayUntil(&next_wake_time, mainTASK_CHATTERBOX_OUTPUT_FREQUENCY/(2 * mainBLINK_PER_TICK));
      gpio_set_level(params->gpio, 1);
    }
    params->elapsed_time++;
    //below releases the useless_load_semaphore, enabling other tasks to use it
    xSemaphoreGive(useless_load_semaphore);
  }
}

static void chatterbox_task(void *v_params) {
  TaskParams *params = (TaskParams *)v_params;
  //Noting the period down in terms of ticks
  TickType_t periodTime = pdMS_TO_TICKS(params->period*1000);
  TickType_t tickCount;
  //vTaskDelay() will cause a task to block for the specified number of ticks from the time vTaskDelay() is called
  //here the pdMS_TO_TICKS is being done to convert time in ms to system ticks to be able to accordingly schedule it
  vTaskDelay(pdMS_TO_TICKS(params->release_time*1000));
  for (int i = 0; true; ++i) {
    if(params->repetitions==0)break;
    //get current tick count to time when the task is released
    tickCount = xTaskGetTickCount();

    gpio_set_level(params->gpio, 1);//1 indicates the LED being turned on
    useless_load(params, params->execution_time);//useless_load as defined ensures the LED blinks for the execution time
    gpio_set_level(params->gpio, 0);//0 indicates the LED being turned off
    
    //delayed from the time when the task was released + the period of the task 
    vTaskDelayUntil(&tickCount,periodTime);
    params->repetitions--;
  }
  //We use this to remove the task from the scheduler so that the scheduler doesn't need to manage this
  //thereby freeing up some of the resources and we pass NULL, to ensure the caller task is killed
  vTaskDelete(NULL);

}


//Red Task Params - Task 1
#define mainTASK_CHATTERBOX_TASK1_PRIORITY (tskIDLE_PRIORITY + 3)
TaskParams task1_params = {.id = 1,
                           .repetitions = -1,
                           .release_time = 1,
                           .execution_time = 1,
                           .period = 3,
                           .gpio = mainTASK_CHATTERBOX_TASK1_GPIO};

//Green Task Params - Task 2
#define mainTASK_CHATTERBOX_TASK2_PRIORITY (tskIDLE_PRIORITY + 2)
TaskParams task2_params = {.id = 2,
                           .repetitions = -1,
                           .release_time = 0,
                           .execution_time = 2,
                           .period = 4,
                           .gpio = mainTASK_CHATTERBOX_TASK2_GPIO};

//Blue Task Params - Task 3
#define mainTASK_CHATTERBOX_TASK3_PRIORITY (tskIDLE_PRIORITY + 1)
TaskParams task3_params = {.id = 3,
                           .repetitions = 3,
                           .release_time = 0,
                           .execution_time = 1,
                           .period = 6,
                           .gpio = mainTASK_CHATTERBOX_TASK3_GPIO};




void app_main(void) {
  /*
  Real-time Concepts for Embedded Systems - Übung  - Assignment 1
  Team Members
  Aashik Muringathery Shoby (3762691)
  Allen Saldanha (3764343)
  Mohamed Bargout (3763454)
  Rico Haas (3310344)  
  */
  

  // gpio configuration
  gpio_config_t io_conf = {.pin_bit_mask = GPIO_PIN_BIT_MASK,
                           .mode = GPIO_MODE_OUTPUT,
                           .pull_up_en = 0,
                           .pull_down_en = 0,
                           .intr_type = GPIO_INTR_DISABLE};
  gpio_config(&io_conf);

  // semaphore to ensure tasks are only preemted after a timeslot
  useless_load_semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(useless_load_semaphore);

   xTaskCreate(chatterbox_task,"Task 1", 2048, &task1_params, mainTASK_CHATTERBOX_TASK1_PRIORITY, NULL);
   xTaskCreate(chatterbox_task,"Task 2", 2048, &task2_params, mainTASK_CHATTERBOX_TASK2_PRIORITY, NULL);
   xTaskCreate(chatterbox_task,"Task 3", 2048, &task3_params, mainTASK_CHATTERBOX_TASK3_PRIORITY, NULL);

  printf("Nothing to see yet\n");
}
