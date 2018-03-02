/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "main.h"

/* Function prototypes -------------------------------------------------------*/
extern void vStartDHCPTask();
extern void vStartADCTask();

void initTask(void const *argument) {
	vStartDHCPTask();
	vStartADCTask();
	osThreadTerminate(NULL);
}

void MX_FREERTOS_Init(void) {
	osThreadDef(Init, initTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(Init), NULL);
}