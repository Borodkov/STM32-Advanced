/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "lcd_log.h"
#include "board_init.h"

extern void MX_FREERTOS_Init(void);

void MX_USB_DEVICE_Init(void);

int main(void) {
  BSP_Init();

  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(LTDC_ACTIVE_LAYER, LCD_FB_START_ADDRESS);
  BSP_LCD_SelectLayer(LTDC_ACTIVE_LAYER);
  BSP_LCD_SetFont(&LCD_DEFAULT_FONT);

  BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
  BSP_TS_ITConfig();
  BSP_TS_ITClear();

  LCD_LOG_Init();
  LCD_LOG_SetHeader("Advanced STM32F746-Discovery");
  LCD_LOG_SetFooter("Press BTN or rotate encoder to switch apps | Load 00%");

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  // TraceRecorder via USB_CDC
  TRC_STREAM_PORT_INIT();
  vTraceEnable(TRC_START_AWAIT_HOST);
  
  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  while (1)
    ;
}

/* USB CDC Descriptors */

extern USBD_DescriptorsTypeDef FS_Desc;

/* USB Device Core handle declaration */
USBD_HandleTypeDef hUsbDeviceFS;

/* init function */
void MX_USB_DEVICE_Init(void) {
    /* Init Device Library,Add Supported Class and Start the library*/
    USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
    USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
    USBD_Start(&hUsbDeviceFS);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM14) {
    HAL_IncTick();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
void _Error_Handler(char * file, int line) {
  /* User can add his own implementation to report the HAL error return state */
  while (1)
    ;
}

#ifdef USE_FULL_ASSERT

/**
   * @brief Reports the name of the source file and the source line number
   * where the assert_param error has occurred.
   * @param file: pointer to the source file name
   * @param line: assert_param error line source number
   * @retval None
   */
void assert_failed(uint8_t* file, uint32_t line) {
  /* User can add his own implementation to report the file name and line number,
    ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
}

#endif
