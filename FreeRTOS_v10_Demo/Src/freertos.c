/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Function prototypes -------------------------------------------------------*/
extern void vStartTimers();
extern void vStartMandelbrotTask();
extern void vUARTCommandConsoleStart();

void led1Task(void const * argument)
{
	for (;;) {
		BSP_LED_Toggle(LED1);
		osDelay(555);
	}
}

void led2Task(void const * argument)
{
	for (;;) {
		BSP_LED_Toggle(LED1);
		osDelay(333);
	}
}

void MX_FREERTOS_Init(void) {
	osThreadDef(LED1, led1Task, osPriorityHigh, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(LED1), NULL);
	osThreadDef(LED2, led2Task, osPriorityAboveNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(LED2), NULL);

	vStartTimers();
	vStartMandelbrotTask();
	vUARTCommandConsoleStart();
}

