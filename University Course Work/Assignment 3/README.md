# Earliest Deadline First App

Project Structure:
```
├── CMakeLists.txt
├── main
│   ├── CMakeLists.txt
│   ├── display.c
│   ├── display.h
│   ├── edf.c               # Subtask 1
│   ├── edf.h
│   ├── idf_component.yml
│   ├── main.c
│   ├── tasks.c             # Subtask 2
│   └── tasks.h
├── README.md
├── sdkconfig
├── sdkconfig.ci
└── sdkconfig.old
```

It is your task to fill in the missing TODOs in *main/edf.c* and *main/tasks.c*.
You should not have to touch any other files.

You can build the project as usual:
```
$ get_idf
$ idf.py build
```

Assuming your ESP32 is visible under /dev/ttyUSB0, you can flash and monitor the chatterbox app with
```
$ idf.py -p /dev/ttyUSB0 flash monitor
```

Finally, to stop the LEDs from blinking, run
```
$ idf.py -p /dev/ttyUSB0 erase-flash
```
