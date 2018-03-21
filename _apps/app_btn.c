/**
  ******************************************************************************
    @file
    @author  PavelB
    @version V1.0
    @date    03-April-2017
    @brief
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "stm32746g_discovery.h"
#include "lcd_log.h"

/* Extern Variables ----------------------------------------------------------*/
extern osThreadId ADCHandle;
extern osThreadId UARTHandle;
extern osThreadId MODBUSHandle;

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/

/**
    @brief
    @param
    @retval None
*/
static void task(void const *pvParameters) {
    uint32_t tikcs;
    LCD_DbgLog("USER BTN: press to contine...\n");

    // w8 user press blue btn
    while (BSP_PB_GetState(BUTTON_KEY) == RESET) {
        osDelay(1);
    }

    LCD_DbgLog("USER BTN: done.\n");
    osThreadResume(ADCHandle);

    /* Infinite loop */
    for (;;) {
        osDelay(125);

        if (BSP_PB_GetState(BUTTON_KEY) == SET) {
            // w8
            tikcs = 0;

            while (BSP_PB_GetState(BUTTON_KEY) == SET && tikcs <= 50) {
                osDelay(10);
                tikcs++;
            }

            // simple press (less then 500 ms)
            if (tikcs < 50) {
                LCD_DbgLog("USER BTN: simple press\n");
                LCD_DbgLog("*!* ModBus Mode *!*\n");
                osThreadSuspend(UARTHandle);
                osThreadResume(MODBUSHandle);
            } else {
                // long press
                // ...
                LCD_DbgLog("USER BTN: long press\n");
                LCD_DbgLog("*!* UART Mode *!*\n");
                osThreadSuspend(MODBUSHandle);
                osThreadResume(UARTHandle);

                // w8 when button relesed
                while (BSP_PB_GetState(BUTTON_KEY) == SET) {
                    osDelay(10);
                } osDelay(100);
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
void vStartBTNTask() {
    LCD_UsrLog("vStartBTNTask()\n");
    /* Create that task */
    osThreadDef(BTN,
                task,
                osPriorityNormal,
                0,
                0x100);
    osThreadCreate(osThread(BTN), NULL);
}
