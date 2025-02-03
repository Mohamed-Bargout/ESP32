#ifndef ICPP_CRITICAL_SECTION_H
#define ICPP_CRITICAL_SECTION_H

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

#define RESOURCES 2
typedef unsigned long PriorityType_t;

typedef struct
{
  SemaphoreHandle_t semaphore;     // Handles access to the resource itself
  PriorityType_t resource_ceiling; // The priority ceiling of the resource
  PriorityType_t last_priority;    // The task accessing the resource stores their
                                   // original priority here to reset it when done
} CriticalSectionSemaphore;

typedef struct
{
  int resource;
  TickType_t start;
  TickType_t end;
  CriticalSectionSemaphore *semaphore;
} CriticalSection;

/* Initialize semaphore for a shared resource.
 * - params: list of periodic tasks params (PeriodicTaskParams*) accessing the
 *	     shared resource
 * - number_of_tasks: size of params array
 */
CriticalSectionSemaphore usPrioritySemaphoreInit(void **params,
                                                 int number_of_tasks);

// Calling task waits for semaphore to be released
void usPrioritySemaphoreWait(CriticalSectionSemaphore *cs_semaphore);
// Calling task released semaphore
void usPrioritySemaphoreSignal(CriticalSectionSemaphore *cs_semaphore);

#endif
