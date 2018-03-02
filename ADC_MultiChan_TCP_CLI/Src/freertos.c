/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "lcd_log.h"
#include "hr_gettime.h"

/* Function prototypes -------------------------------------------------------*/
extern void vStartTimers();
extern void vStartDHCPTask();
extern void vStartViewStringTask();
extern void vStartADCTask();
extern void vStartBTNTask();
extern void vStartModBusTask();
extern void vStartUARTTask();
extern void vStartFractalTask();
extern void vStartEncoderTask();
extern void vStartJPEGTask();
extern void vStartMandelbrotTask();
extern void vStartSlideShowTask();

void initTask(void const * argument)
{
   vStartTimers();

   vStartDHCPTask();
	 
  // vStartViewStringTask();
  // vStartADCTask();
  // vStartBTNTask();
  // vStartModBusTask();
  // vStartUARTTask();
  // vStartEncoderTask();
  // vStartFractalTask();
  // vStartJPEGTask();
  // vStartMandelbrotTask();
  vStartSlideShowTask();
	
  osThreadTerminate(NULL);
}

void MX_FREERTOS_Init(void) {
  osThreadDef(Init, initTask, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  osThreadCreate(osThread(Init), NULL);
}

/* Functions needed when configGENERATE_RUN_TIME_STATS is on */
void configureTimerForRunTimeStats(void) {
  vStartHighResolutionTimer();
}

uint32_t getRunTimeCounterValue(void) {
  static uint64_t ullHiresTime = 0; /* Is always 0? */

  return ( uint32_t ) ( ullGetHighResolutionTime() - ullHiresTime );
}

__weak void vApplicationStackOverflowHook(xTaskHandle xTask, signed char *pcTaskName) {
   /* Run time stack overflow checking is performed if
   configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2. This hook function is
   called if a stack overflow is detected. */

  ( void ) pcTaskName;
  ( void ) xTask;

  /* Run time stack overflow checking is performed if
  configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
  function is called if a stack overflow is detected. */
  taskDISABLE_INTERRUPTS();
  LCD_UsrLog("*********************\n");
  LCD_UsrLog("Shit! Stack overflow!\n");
  LCD_UsrLog("*********************\n");
  for( ;; );
}

__weak void vApplicationMallocFailedHook(void) {
   /* vApplicationMallocFailedHook() will only be called if
   configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h. It is a hook
   function that will get called if a call to pvPortMalloc() fails.
   pvPortMalloc() is called internally by the kernel whenever a task, queue,
   timer or semaphore is created. It is also called by various parts of the
   demo application. If heap_1.c or heap_2.c are used, then the size of the
   heap available to pvPortMalloc() is defined by configTOTAL_HEAP_SIZE in
   FreeRTOSConfig.h, and the xPortGetFreeHeapSize() API function can be used
   to query the size of free heap space that remains (although it does not
   provide information on how the remaining heap might be fragmented). */
  
  volatile uint32_t ulMallocFailures = 0;

  /* Called if a call to pvPortMalloc() fails because there is insufficient
  free memory available in the FreeRTOS heap.  pvPortMalloc() is called
  internally by FreeRTOS API functions that create tasks, queues, software
  timers, and semaphores.  The size of the FreeRTOS heap is set by the
  configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */
  ulMallocFailures++;
}

/**
  * @brief  Pre Sleep Processing
  * @param  ulExpectedIdleTime: Expected time in idle state
  * @retval None
  */
void PreSleepProcessing(uint32_t * ulExpectedIdleTime)
{
  /* Called by the kernel before it places the MCU into a sleep mode because
  configPRE_SLEEP_PROCESSING() is #defined to PreSleepProcessing().

  NOTE:  Additional actions can be taken here to get the power consumption
  even lower.  For example, peripherals can be turned off here, and then back
  on again in the post sleep processing function.  For maximum power saving
  ensure all unused pins are in their lowest power state. */

  /* Disable the peripheral clock during Low Power (Sleep) mode.*/
  __HAL_RCC_GPIOG_CLK_SLEEP_DISABLE();
  /* 
    (*ulExpectedIdleTime) is set to 0 to indicate that PreSleepProcessing contains
    its own wait for interrupt or wait for event instruction and so the kernel vPortSuppressTicksAndSleep 
    function does not need to execute the wfi instruction  
  */
  *ulExpectedIdleTime = 0;
  
  /*Enter to sleep Mode using the HAL function HAL_PWR_EnterSLEEPMode with WFI instruction*/
  HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);  
}

/**
  * @brief  Post Sleep Processing
  * @param  ulExpectedIdleTime: Not used
  * @retval None
  */
void PostSleepProcessing(uint32_t * ulExpectedIdleTime)
{
  /* Called by the kernel when the MCU exits a sleep mode because
  configPOST_SLEEP_PROCESSING is #defined to PostSleepProcessing(). */

  /* Avoid compiler warnings about the unused parameter. */
  (void) ulExpectedIdleTime;
}