/*
 * FreeRTOS Kernel V10.2.0
 * Copyright (C) 2019 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */

/* 
	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used.
*/


/*
 * Creates all the demo application tasks, then starts the scheduler.  The WEB
 * documentation provides more details of the demo application tasks.
 * 
 * Main.c also creates a task called "Check".  This only executes every three 
 * seconds but has the highest priority so is guaranteed to get processor time.  
 * Its main function is to check that all the other tasks are still operational.
 * Each task (other than the "flash" tasks) maintains a unique count that is 
 * incremented each time the task successfully completes its function.  Should 
 * any error occur within such a task the count is permanently halted.  The 
 * check task inspects the count of each task to ensure it has changed since
 * the last time the check task executed.  If all the count variables have 
 * changed all the tasks are still executing error free, and the check task
 * toggles the onboard LED.  Should any task contain an error at any time 
 * the LED toggle rate will change from 3 seconds to 500ms.
 *
 */

/* Standard includes. */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "lpc21xx.h"

/* Peripheral includes. */
#include "serial.h"
#include "GPIO.h"


/*-----------------------------------------------------------*/

/* Constants to setup I/O and processor. */
#define mainBUS_CLK_FULL	( ( unsigned char ) 0x01 )

/* Constants for the ComTest demo application tasks. */
#define mainCOM_TEST_BAUD_RATE	( ( unsigned long ) 115200 )

#define BUTTON_1_MONITOR_P 50
#define BUTTON_2_MONITOR_P 50
#define PERIODIC_TRANSMITTER_P 100
#define UART_RECEIVER_P 20
#define LOAD_1_SIMULATION_P 10
#define LOAD_2_SIMULATION_P 100

#define PRESSED_STATE 1
#define RELEASED_STATE 2

TaskHandle_t Button_1_Monitor_h = NULL;
TaskHandle_t Button_2_Monitor_h = NULL;
TaskHandle_t Periodic_Transmitter_h = NULL;
TaskHandle_t Uart_Receiver_h = NULL;
TaskHandle_t Load_1_Simulation_h = NULL;
TaskHandle_t Load_2_Simulation_h = NULL;

QueueHandle_t Queue1;
const char HelloCmd[] = "Hello\n\r";
const char Btn1Pressed[] = "Btn1 Pressed\n\r";
const char Btn1Released[] = "Btn1 Released\n\r";
const char Btn2Pressed[] = "Btn2 Pressed\n";
const char Btn2Released[] = "Btn2 Released\n";


uint32_t Button_1_in_time, Button_1_time;
uint32_t Button_2_in_time, Button_2_time;
uint32_t Periodic_Transmitter_in_time, Periodic_Transmitter_time;
uint32_t Uart_Receiver_in_time, Uart_Receiver_time;
uint32_t Load_1_Simulation_in_time, Load_1_Simulation_time;
uint32_t Load_2_Simulation_in_time, Load_2_Simulation_time;
uint32_t CurrentTime, SystemTime, CPU_Load;

uint32_t misses;

/*
 * Configure the processor for use with the Keil demo board.  This is very
 * minimal as most of the setup is managed by the settings in the project
 * file.
 */
static void prvSetupHardware( void );
/*-----------------------------------------------------------*/

void Button_1_Monitor( void * pvParameters );
void Button_2_Monitor( void * pvParameters );
void Periodic_Transmitter( void * pvParameters );
void Uart_Receiver( void * pvParameters );
void Load_1_Simulation( void * pvParameters );
void Load_2_Simulation( void * pvParameters );
void vApplicationTickHook( void );
void vApplicationIdleHook( void );

/*
 * Application entry point:
 * Starts all the other tasks, then starts the scheduler. 
 */
int main( void )
{
	/* Setup the hardware for use with the Keil demo board. */
	prvSetupHardware();

		
	xTaskCreate(
							Button_1_Monitor,       /* Function that implements the task. */
							"Button 1",          /* Text name for the task. */
							200,      /* Stack size in words, not bytes. */
							( void * ) 0,    /* Parameter passed into the task. */
							2,/* Priority at which the task is created. */
							#if ( configUSE_EDF_SCHEDULER == 1 )
							 &Button_1_Monitor_h, 
							(TickType_t)BUTTON_1_MONITOR_P); //period
							#else
							&Button_1_Monitor_h); 
							#endif

		xTaskCreate(
							Button_2_Monitor,       /* Function that implements the task. */
							"Button 2",          /* Text name for the task. */
							200,      /* Stack size in words, not bytes. */
							( void * ) 0,    /* Parameter passed into the task. */
							1,/* Priority at which the task is created. */
							#if ( configUSE_EDF_SCHEDULER == 1 )
							 &Button_2_Monitor_h, 
							(TickType_t)BUTTON_2_MONITOR_P); //period
							#else
							&Button_2_Monitor_h); 
							#endif

		xTaskCreate(
							Periodic_Transmitter,       /* Function that implements the task. */
							"Transmitter",          /* Text name for the task. */
							200,      /* Stack size in words, not bytes. */
							( void * ) 0,    /* Parameter passed into the task. */
							1,/* Priority at which the task is created. */
							#if ( configUSE_EDF_SCHEDULER == 1 )
							 &Periodic_Transmitter_h, 
							(TickType_t)PERIODIC_TRANSMITTER_P); //period
							#else
							&Periodic_Transmitter_h); 
							#endif
	
		xTaskCreate(
							Uart_Receiver,       /* Function that implements the task. */
							"Receiver",          /* Text name for the task. */
							200,      /* Stack size in words, not bytes. */
							( void * ) 0,    /* Parameter passed into the task. */
							1,/* Priority at which the task is created. */
							#if ( configUSE_EDF_SCHEDULER == 1 )
							 &Uart_Receiver_h, 
							(TickType_t)UART_RECEIVER_P); //period
							#else
							&Uart_Receiver_h); 
							#endif

		xTaskCreate(
							Load_1_Simulation,       /* Function that implements the task. */
							"Load_1_Simulation",          /* Text name for the task. */
							200,      /* Stack size in words, not bytes. */
							( void * ) 0,    /* Parameter passed into the task. */
							1,/* Priority at which the task is created. */
							#if ( configUSE_EDF_SCHEDULER == 1 )
							 &Load_1_Simulation_h, 
							(TickType_t)LOAD_1_SIMULATION_P); //period
							#else
							&Load_1_Simulation_h); 
							#endif
							
		xTaskCreate(
							Load_2_Simulation,       /* Function that implements the task. */
							"Load_2_Simulation",          /* Text name for the task. */
							200,      /* Stack size in words, not bytes. */
							( void * ) 0,    /* Parameter passed into the task. */
							1,/* Priority at which the task is created. */
							#if ( configUSE_EDF_SCHEDULER == 1 )
							 &Load_2_Simulation_h, 
							(TickType_t)LOAD_2_SIMULATION_P); //period
							#else
							&Load_2_Simulation_h); 
							#endif
							
		Queue1 = xQueueCreate(200, sizeof(uint8_t));


	/* Now all the tasks have been started - start the scheduler.

	NOTE : Tasks run in system mode and the scheduler runs in Supervisor mode.
	The processor MUST be in supervisor mode when vTaskStartScheduler is 
	called.  The demo applications included in the FreeRTOS.org download switch
	to supervisor mode prior to main being called.  If you are not using one of
	these demo application projects then ensure Supervisor mode is used here. */
	vTaskStartScheduler();

	/* Should never reach here!  If you do then there was not enough heap
	available for the idle task to be created. */
	for( ;; );
}
/*-----------------------------------------------------------*/

void Button_1_Monitor( void * pvParameters )
{
	static uint8_t state = RELEASED_STATE;
	static uint32_t start_time, end_time;
	pinState_t level;
	int i;
	TickType_t xLastWakeTime;
	const TickType_t period =  BUTTON_1_MONITOR_P / portTICK_PERIOD_MS;
	
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void*)2);
	
	
	for( ;; )
	{
		start_time = xTaskGetTickCount();
		level = GPIO_read(PORT_0, PIN6);
		if (state == RELEASED_STATE && level == PIN_IS_HIGH) {
			for(i = 0; i < strlen(Btn1Pressed); i++)
			{
				xQueueSend(Queue1, &Btn1Pressed[i], 0);
			}
			state = PRESSED_STATE;
		} else if(state == PRESSED_STATE && level == PIN_IS_LOW) {
			for(i = 0; i < strlen(Btn1Released); i++)
			{
				xQueueSend(Queue1, &Btn1Released[i], 0);
			}
			state = RELEASED_STATE;
		}
		end_time = xTaskGetTickCount();
		if((end_time - start_time) > BUTTON_1_MONITOR_P)
		{
			misses++;
		}
		
		vTaskDelayUntil( &xLastWakeTime, period);
	}
}

void Button_2_Monitor( void * pvParameters )
{
	static uint8_t state = RELEASED_STATE;
	static uint32_t start_time, end_time;
	
	pinState_t level;
	int i;
	
	TickType_t xLastWakeTime;
	const TickType_t period =  BUTTON_2_MONITOR_P / portTICK_PERIOD_MS;
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void*)3);

	for( ;; )
	{
		start_time = xTaskGetTickCount();

		level = GPIO_read(PORT_0, PIN7);
		if (state == RELEASED_STATE && level == PIN_IS_HIGH) {
			for(i = 0; i < strlen(Btn2Pressed); i++)
			{
				xQueueSend(Queue1, &Btn2Pressed[i], 0);
			}
			state = PRESSED_STATE;
		} else if(state == PRESSED_STATE && level == PIN_IS_LOW) {
			for(i = 0; i < strlen(Btn2Released); i++)
			{
				xQueueSend(Queue1, &Btn2Released[i], 0);
			}
			state = RELEASED_STATE;
		}
		end_time = xTaskGetTickCount();
		if((end_time - start_time) > BUTTON_2_MONITOR_P)
		{
			misses++;
		}
		vTaskDelayUntil( &xLastWakeTime, period );
	}
}

void Periodic_Transmitter( void * pvParameters )
{
	static uint32_t start_time, end_time;
	
	TickType_t xLastWakeTime;
	const TickType_t period =  PERIODIC_TRANSMITTER_P / portTICK_PERIOD_MS;
	int i;
	
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void*)4);

	for( ;; )
	{
		start_time = xTaskGetTickCount();

		for(i = 0; i < strlen(HelloCmd); i++)
			{
				xQueueSend(Queue1, &HelloCmd[i], 0);
			}
			
		end_time = xTaskGetTickCount();
		if((end_time - start_time) > PERIODIC_TRANSMITTER_P)
		{
			misses++;
		}
		
		vTaskDelayUntil( &xLastWakeTime, period );
	}
}

void Uart_Receiver( void * pvParameters )
{
	static uint32_t start_time, end_time;
	TickType_t xLastWakeTime;
	const TickType_t period =  UART_RECEIVER_P / portTICK_PERIOD_MS;
	uint8_t rx;

	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void*)5);

	for( ;; )
	{
		start_time = xTaskGetTickCount();

		// Print any new messages from Queue 1
		while(xQueueReceive(Queue1, &rx, 0) == pdTRUE)
		{
			xSerialPutChar((signed char)rx);
		}
		end_time = xTaskGetTickCount();
		if((end_time - start_time) > UART_RECEIVER_P)
		{
			misses++;
		}
		vTaskDelayUntil( &xLastWakeTime, period );
	}
}

void Load_1_Simulation( void * pvParameters )
{
	static uint32_t start_time, end_time;

	TickType_t xLastWakeTime;
	const TickType_t period =  LOAD_1_SIMULATION_P / portTICK_PERIOD_MS;
	int i;
	
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void*)6);

	for( ;; )
	{
		start_time = xTaskGetTickCount();

		for(i = 0; i < 37000; i++);
		end_time = xTaskGetTickCount();
		if((end_time - start_time) > LOAD_1_SIMULATION_P)
		{
			misses++;
		}
		vTaskDelayUntil( &xLastWakeTime, period );
	}
}

void Load_2_Simulation( void * pvParameters )
{
	static uint32_t start_time, end_time;

	TickType_t xLastWakeTime;
	const TickType_t period =  LOAD_2_SIMULATION_P / portTICK_PERIOD_MS;
	int i;
	
	xLastWakeTime = xTaskGetTickCount();
	vTaskSetApplicationTaskTag(NULL, (void*)7);

	for( ;; )
	{
		start_time = xTaskGetTickCount();
		for(i = 0; i < 85000; i++);
		end_time = xTaskGetTickCount();
		if((end_time - start_time) > LOAD_2_SIMULATION_P)
		{
			misses++;
		}
		vTaskDelayUntil( &xLastWakeTime, period );
	}
}

void vApplicationTickHook( void )
{
	GPIO_write(PORT_1, PIN0, PIN_IS_HIGH);
	GPIO_write(PORT_1, PIN0, PIN_IS_LOW);
}


/* Function to reset timer 1 */
void timer1Reset(void)
{
	T1TCR |= 0x2;
	T1TCR &= ~0x2;
}

/* Function to initialize and start timer 1 */
static void configTimer1(void)
{
	T1PR = 1000;
	T1TCR |= 0x1;
}

static void prvSetupHardware( void )
{
	/* Perform the hardware setup required.  This is minimal as most of the
	setup is managed by the settings in the project file. */

	/* Configure UART */
	xSerialPortInitMinimal(mainCOM_TEST_BAUD_RATE);

	/* Configure GPIO */
	GPIO_init();
	
	/* Config trace timer 1 and read T1TC to get current tick */
	configTimer1();

	/* Setup the peripheral bus to be the same as the PLL output. */
	VPBDIV = mainBUS_CLK_FULL;
}
/*-----------------------------------------------------------*/


