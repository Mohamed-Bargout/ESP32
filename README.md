# ESP32 Projects

This repository contains various ESP32 projects, including a series of assignments related to real-time concepts.

## Folder Structure
```
├── Real Time Concepts Course/
│   ├── Assignment 1/
│   │   └── main/
│   │       └── main.c
│   ├── Assignment 2/
│   │   └── main/
│   │       └── main.c
│   ├── Assignment 3/
│   │   └── main/
│   │       └── main.c
│   └── Assignment 4/
│       └── main/
│           └── main.c
```

## Real Time Concepts Assignments

This section details the assignments completed as part of the "Real Time Concepts" Course at the University of Stuttgart, which focuses on using FreeRTOS to implement real-time systems.

*   **Assignment 1: Basic Task Scheduling with LEDs:** This assignment introduces basic task scheduling in FreeRTOS.  The goal is to implement three tasks, each associated with an LED.  Each task has a defined release time, period, execution time, and number of repetitions. The LEDs provide a visual representation of the task states:
    *   **LED OFF:** Task is not ready to execute.
    *   **LED ON (Steady):** Task is ready to execute.
    *   **LED Blinking:** Task is currently executing.

*   **Assignment 2: Real-Time Schedulability Analysis:** This assignment focuses on analyzing the schedulability of real-time tasks. Three different schedulability checks were implemented:
    *   **Schedulability Checks:** Three distinct methods for assessing task schedulability were implemented:
        *   **Time Demand Analysis:** This method analyzes the total time a task requires to execute, considering interference from higher-priority tasks.
        *   **Worst-Case Simulation:** This approach simulates the execution of tasks under worst-case scenarios to verify that deadlines are still met.
        *   **Utilization Bound Test:** This test provides a sufficient (but not necessary) condition for schedulability based on the overall utilization of the CPU by the tasks.
    *   **Acceptance Test:**  A crucial part of this assignment is the development of an acceptance test.  This test determines whether a new task can be safely added to the existing task set without violating the timing guarantees of any higher-priority tasks.  The acceptance test combines the results from the Time Demand Analysis, Worst-Case Simulation, and Utilization Bound Test to make this determination.
    *   **SSD1306 Display Integration:** The display shows the results of each of the schedulability tests (Time Demand Analysis, Worst-Case Simulation, and Utilization Bound Test) for every task that is currently running in the system.

*   *   **Assignment 3: EDF Scheduling with a Deferrable Server:** This assignment implements an EDF (Earliest Deadline First) scheduler and integrates a deferrable server as well as implementing a system density test.  
    *   **System Density Test:** The schedulability of the system is assessed using a density-based criterion. The test iterates through periodic tasks and the deferrable server, ensuring that all accepted tasks can be scheduled without violating real-time constraints.  
    *   **Deferrable Server Implementation:** A deferrable server is added to handle aperiodic tasks efficiently. When an aperiodic task request is triggered via a button press, the server executes it within its allocated budget. If no aperiodic tasks are pending, the remaining budget is returned to the EDF scheduler.  
    *   **SSD1306 Display Integration:** The execution of aperiodic tasks is visually represented on an SSD1306 display, where each execution triggers an animation.  

*   **Assignment 4:** 

## Building and Running the Assignments

Each assignment is contained within its own folder. To build and run an assignment, follow these steps:

1.  **Install ESP-IDF:**  Make sure you have the ESP-IDF installed on your system.

2.  **Export ESP-IDF Path:** Open a terminal and export the ESP-IDF path:

    ```bash
    . $HOME/esp/esp-idf/export.sh
    ```

    (Adjust the path if your ESP-IDF installation directory is different.)

3.  **Navigate to Assignment Folder:** Change the directory to the specific assignment folder you want to build:

    ```bash
    cd "Real Time Concepts Course/Assignment 1"
    ```

4.  **Set Target:** Set the target to esp32:

    ```bash
    idf.py set-target esp32
    ```

5.  **Configure FreeRTOS:** Configure the project to enable FreeRTOS:

    ```bash
    idf.py menuconfig
    ```

6.  **Build:** Build the project:

    ```bash
    idf.py build
    ```

7.  **Flash:** Flash the binaries to your ESP32 board. Replace `PORT` with the actual serial port your ESP32 is connected to (`/dev/ttyUSB0` or `COM3`):

    ```bash
    idf.py -p PORT flash
    ```

8.  **Monitor:** Open the serial monitor to view the output from your ESP32:

    ```bash
    idf.py -p PORT monitor
    ```
