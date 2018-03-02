/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "main.h"

void led1Task(void const * argument)
{
	for (;;) {
		BSP_LED_On(LED1);
		osDelay(100);
	}
}

void led2Task(void const * argument)
{
	for (;;) {
		BSP_LED_Off(LED1);
		osDelay(50);
	}
}

void MX_FREERTOS_Init(void) {
	osThreadDef(LED1, led1Task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(LED1), NULL);
	osThreadDef(LED2, led2Task, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(LED2), NULL);
}
