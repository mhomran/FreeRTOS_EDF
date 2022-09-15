# About

This repository contains an implementation of the Earliest Deadline First (EDF) scheduler for FreeRTOS 10.3.1v. The FreeRTOS
version is modified so that the kernel can schedule tasks not using its fixed priority scheduler but with a dynamic priority one; the EDF scheduler.


# Core Source files

`task.c`: a modified version of `task.c` of the FreeRTOS. This file contains the task control block (TCB) and the FreeRTOS
scheduler.

`task.h`: a modified version of `task.h` of the FreeRTOS. The header file of `task.c`.

`FreeRTOSConfig.h`: this configuration of FreeRTOS. The configuration contains a macro to enable or disable the EDF
scheduler and return to the default scheduler.

# Other Source files

`serial.c`: a library for UART communication for the NXP LPC2129.

`serial.h`: a header file of `serial.c`.

`main.c`: an application for the NXP LPC2129. You can find  the requirements of
 the application in the doc folder.

# How to use the scheduler ?

1. It's easy to use that scheduler, you just need to replace `task.c` and `task.h` with the default ones. 
2. Then, add this line `#define configUSE_EDF_SCHEDULER   1`  to your `FreeRTOSConfig.h`. 
3. Compile your project. If you can compile your FreeRTOS files successfully, then hurray, you got it !
