#include "critical_section.h"
#include "tasks.h"
#include <stdio.h>
#include <stdbool.h>

/*
  Real-time Concepts for Embedded Systems - Übung  - Assignment 4
  Team Members
  Aashik Muringathery Shoby (3762691)
  Allen Saldanha (3764343)
  Mohamed Bargout (3763454)
  Rico Haas (3310344)
  */

#define MAX_TASK_COUNT 20
#define MAX_RESOURCE_COUNT 20
#define ADDITIONAL_DEBUG_MESSAGES false

// Stores references related to a single task to manage global resource access
typedef struct
{
  PeriodicTaskParams *task_params;   // Task metainformation
  SemaphoreHandle_t task_semaphore;  // The task waits for this semaphore when blocked due to ceilings
  PriorityType_t *task_ceiling_list; // Ceilings of all the resources the task has
} TaskProfile;

// Stores information about tasks to manage global resource access
static TaskProfile task_profiles[MAX_TASK_COUNT];
static size_t task_profiles_size = 0;
// Stores all ceilings in the system that are currently active
static PriorityType_t active_system_ceilings[MAX_RESOURCE_COUNT] = {tskIDLE_PRIORITY};

// Find the task profile of the current task
TaskProfile *getCurrentTaskProfile()
{
  TaskHandle_t currentTaskHandle = xTaskGetCurrentTaskHandle();
  for (int i = 0; i < task_profiles_size; i++)
  {
    if (task_profiles[i].task_params->handle == currentTaskHandle)
    {
      return &task_profiles[i];
    }
  }
  printf("ERROR: Unable to receive task profile!\n");
  return NULL;
}

// Add a ceiling to a ceiling list
void addToCeilingList(PriorityType_t list[MAX_RESOURCE_COUNT], PriorityType_t value)
{
  for (int i = 0; i < MAX_RESOURCE_COUNT; i++)
  {
    if (list[i] == tskIDLE_PRIORITY)
    {
      list[i] = value;
      return;
    }
  }
  printf("ERROR: Ceiling list full!\n");
}

// Remove a ceiling from a ceiling list
void removeFromCeilingList(PriorityType_t list[MAX_RESOURCE_COUNT], PriorityType_t value)
{
  for (int i = 0; i < MAX_RESOURCE_COUNT; i++)
  {
    if (list[i] == value)
    {
      list[i] = tskIDLE_PRIORITY;
      return;
    }
  }
  printf("ERROR: Unable to remove ceiling list element %lu\n", value);
}

// Find the highest value in a ceiling list
PriorityType_t getMaxCeilingListValue(PriorityType_t list[MAX_RESOURCE_COUNT])
{
  int maxValue = tskIDLE_PRIORITY;
  for (int i = 0; i < MAX_RESOURCE_COUNT; i++)
  {
    if (list[i] > maxValue)
    {
      maxValue = list[i];
    }
  }
  return maxValue;
}

// Compare two task profiles by their task priority
int compareTaskPriority(const void *a, const void *b)
{
  TaskProfile *profileA = (TaskProfile *)a;
  TaskProfile *profileB = (TaskProfile *)b;
  return ((int)profileB->task_params->priority) - ((int)profileA->task_params->priority);
}

// Releases all tasks blocked by ceilings in order of priority
void releaseAllCeilingBlocks()
{
  // Sort by priority
  qsort(task_profiles, task_profiles_size, sizeof(TaskProfile), compareTaskPriority);
  // Release all
  for (size_t i = 0; i < task_profiles_size; i++)
  {
    xSemaphoreGive(task_profiles[i].task_semaphore);
    if (ADDITIONAL_DEBUG_MESSAGES)
      printf("Task %s ceiling-unblocked\n", task_profiles[i].task_params->id);
  }
}

CriticalSectionSemaphore usPrioritySemaphoreInit(void **params,
                                                 int number_of_tasks)
{
  // Globally store task information, but only one global entry per task
  for (int i = 0; i < number_of_tasks; i++)
  {
    PeriodicTaskParams *param = (PeriodicTaskParams *)params[i];
    // Search whether there's already a record of that task present
    bool task_already_recorded = false;
    for (int i = 0; i < task_profiles_size; i++)
    {
      if (task_profiles[i].task_params->id == param->id)
      {
        task_already_recorded = true;
        break;
      }
    }
    if (!task_already_recorded)
    {
      // Create new task profile
      TaskProfile *new_task_profile = &task_profiles[task_profiles_size++];
      new_task_profile->task_params = param;
      new_task_profile->task_semaphore = xSemaphoreCreateBinary();
      // Properly allocate memory so that the list won't get corrupted when the local scope ends
      new_task_profile->task_ceiling_list = (PriorityType_t *)pvPortMalloc(MAX_RESOURCE_COUNT * sizeof(PriorityType_t));
      if (new_task_profile->task_ceiling_list == NULL)
      {
        printf("ERROR: Memory allocation for task_ceiling_list failed\n");
      }
      for (int i = 0; i < MAX_RESOURCE_COUNT; i++)
      {
        new_task_profile->task_ceiling_list[i] = tskIDLE_PRIORITY;
      }
      if (ADDITIONAL_DEBUG_MESSAGES)
        printf("Task %s added to global list of tasks\n", param->id);
    }
  }
  CriticalSectionSemaphore cs_semaphore;
  // Initialize resource semaphore
  cs_semaphore.semaphore = xSemaphoreCreateBinary();
  xSemaphoreGive(cs_semaphore.semaphore);
  // Determine resource ceiling
  cs_semaphore.resource_ceiling = tskIDLE_PRIORITY;
  for (int i = 0; i < number_of_tasks; i++)
  {
    PeriodicTaskParams *param = (PeriodicTaskParams *)params[i];
    if (param->priority > cs_semaphore.resource_ceiling)
    {
      cs_semaphore.resource_ceiling = param->priority;
    }
  }
  if (ADDITIONAL_DEBUG_MESSAGES)
    printf("The resource ceiling is %lu\n", cs_semaphore.resource_ceiling);
  return cs_semaphore;
}

void usPrioritySemaphoreWait(CriticalSectionSemaphore *cs_semaphore)
{
  TaskProfile *current_task_profile = getCurrentTaskProfile();

  while (true)
  {
    // Do regular blocking when resource not available
    xSemaphoreTake(cs_semaphore->semaphore, portMAX_DELAY);
    // Check ceilings
    PriorityType_t system_ceiling = getMaxCeilingListValue(active_system_ceilings);
    // 1. Is my priority higher than the system ceiling?
    if (uxTaskPriorityGet(NULL) > system_ceiling)
    {
      if (ADDITIONAL_DEBUG_MESSAGES)
        printf("Resource access granted due to task prio superiority\n");
      break;
    }
    // 2. If not, do I hold a resource with a ceiling equal to the system ceiling (except this one)?
    if (getMaxCeilingListValue(current_task_profile->task_ceiling_list) == system_ceiling)
    {
      if (ADDITIONAL_DEBUG_MESSAGES)
        printf("Resource access granted due to superior resource held\n");
    }
    // If none is true:
    if (ADDITIONAL_DEBUG_MESSAGES)
      printf("Resource access denied; ceiling-block\n");
    // Give up the resource semaphore again
    xSemaphoreGive(cs_semaphore->semaphore);
    // Empty your global task semaphore and wait in it
    if (uxSemaphoreGetCount(current_task_profile->task_semaphore) == 1)
    {
      xSemaphoreTake(current_task_profile->task_semaphore, portMAX_DELAY);
    }
    xSemaphoreTake(current_task_profile->task_semaphore, portMAX_DELAY);
    // When released, try to access the resource again (loop to start)
  }

  // When access granted:
  // Store current priority in resource semaphore
  cs_semaphore->last_priority = uxTaskPriorityGet(NULL);
  // Task's priority becomes the resource's ceiling
  if (ADDITIONAL_DEBUG_MESSAGES)
    printf("Task priority set to %lu\n", cs_semaphore->resource_ceiling);
  vTaskPrioritySet(NULL, cs_semaphore->resource_ceiling);
  current_task_profile->task_params->priority = cs_semaphore->resource_ceiling;
  // Add resource ceiling to active system ceilings
  addToCeilingList(active_system_ceilings, cs_semaphore->resource_ceiling);
  if (ADDITIONAL_DEBUG_MESSAGES)
    printf("System ceiling is %lu\n", getMaxCeilingListValue(active_system_ceilings));
  // Add resource ceiling to held ceilings
  addToCeilingList(current_task_profile->task_ceiling_list, cs_semaphore->resource_ceiling);
  if (ADDITIONAL_DEBUG_MESSAGES)
    printf("Resource ceiling is %lu\n", getMaxCeilingListValue(current_task_profile->task_ceiling_list));
  // ACCESS RESOURCE
}

void usPrioritySemaphoreSignal(CriticalSectionSemaphore *cs_semaphore)
{
  TaskProfile *current_task_profile = getCurrentTaskProfile();
  // Remove resource from held ceilings
  removeFromCeilingList(current_task_profile->task_ceiling_list, cs_semaphore->resource_ceiling);
  if (ADDITIONAL_DEBUG_MESSAGES)
    printf("Resource ceiling is %lu\n", getMaxCeilingListValue(current_task_profile->task_ceiling_list));
  // Remove resource from active system ceilings
  removeFromCeilingList(active_system_ceilings, cs_semaphore->resource_ceiling);
  if (ADDITIONAL_DEBUG_MESSAGES)
    printf("System ceiling is %lu\n", getMaxCeilingListValue(active_system_ceilings));
  // Release resource semaphore
  xSemaphoreGive(cs_semaphore->semaphore);
  // Wake all global semaphores in their task priority order
  releaseAllCeilingBlocks();

  // Reset your priority to the value stored in the semaphore earlier
  if (ADDITIONAL_DEBUG_MESSAGES)
    printf("Task priority set to %lu\n", cs_semaphore->last_priority);
  current_task_profile->task_params->priority = cs_semaphore->last_priority;
  vTaskPrioritySet(NULL, cs_semaphore->last_priority);
}
