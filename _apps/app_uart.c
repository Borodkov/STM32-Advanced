/**
  ******************************************************************************
  * @file
  * @author  PavelB
  * @version V1.0
  * @date    03-April-2017
  * @brief
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "stm32746g_discovery.h"
#include "lcd_log.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osThreadId UARTHandle;
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief
  * @param
  * @retval None
  */
static void task(void const *pvParameters) {
  osThreadSuspend(NULL);

  /* Infinite loop */
  for (;;) {
    osDelay(1000);
//    stringCatAndView(sDateTemp);
//
//    strcat(sDateTemp,"\r");
//    BSP_COM_Transmit(COM1, sDateTemp);
  }
}

/*----------------------------------------------------------------------------*/
void vStartUARTTask() {
  LCD_UsrLog("vStartUARTTask()\n");
  /* Create that task */
  osThreadDef(UART,
              task,
              osPriorityNormal,
              0,
              0x100);
  osThreadCreate(osThread(UART), NULL);
}
