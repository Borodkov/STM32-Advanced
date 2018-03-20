/**
  ******************************************************************************
  * File Name   : main.c
  * Description : This example provides a description of how to use a DMA channel
  *               to transfer a word data buffer from FLASH memory to embedded
  *               SRAM memory through the STM32F7xx HAL API.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include  "lcd_log.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NUM_SAMPLES 500
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t result;

volatile uint32_t array1[512];   // in internal DTCM SRAM
volatile uint32_t array2[512];  // in internal DTCM SRAM

uint32_t time_start, time_end;
volatile uint32_t time_diff;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
static void printConfig(void);
static void PLL_M_Config(void);
uint32_t fce(void);
// volatile uint8_t dummy;

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {
  uint32_t n;

  HAL_InitTick(TICK_INT_PRIORITY);

  // initialize the array with some values
  for (n = 0; n < NUM_SAMPLES; n++) {
    array1[n] = n;
    array2[n] = n;
  }

  result = 0;

  MPU_Config(); // configure the memory region
  printConfig();
  printf(" CPU cycles spent for sum of %d values \n", NUM_SAMPLES);
  HAL_SYSTICK_Config(0xFFFFFF);

  // jump to 200MHz
  SystemClock_Config();

  // now enable the caches
  SCB_EnableICache();
  __HAL_FLASH_ART_ENABLE();

  TIM_MEASURE_START;
  result = fce();
  TIM_MEASURE_END;

  printf("WS: %2d, ART: %s, D_cache: %s, I_cache: %s\n", FLASH->ACR & 0xF, (FLASH->ACR & 0x200) ? "ON " : "OFF", ((SCB->CCR & 0x10000) ? "ON " : "OFF"), ((SCB->CCR & 0x20000) ? "ON " : "OFF"));
  printf(" System Clock: %9d, cycles: %d\n", HAL_RCC_GetSysClockFreq(), time_diff);
  printf(" result of computation  : %d\n", result);

  BSP_LCD_DisplayOn();

  while (1);

}

uint32_t fce(void) {
  uint32_t acc, n;
  acc = 0;

  for (n = 0; n < NUM_SAMPLES; n++) {
    acc += ((3 * (array1[n] & 0x0F) + 65) * (25 * (array2[n + 1] & 0x0F) + 71)) ;
  }

  return acc;
}

/**
  * @brief  Configure the MPU attributes as Write Through for SRAM1/2.
  * @note   The Base Address is 0x20010000 since this memory interface is the AXI.
  *         The Region Size is 256KB, it is related to SRAM1 and SRAM2  memory size.
  * @param  None
  * @retval None
  */
static void MPU_Config(void) {
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

//#define DCACHE_WBWA

#ifdef  DCACHE_WBWA
  // these 3 parameters configure Cache properties for this region - write back, write allocate
  MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
#else
  // these 3 parameters configure Cache properties for this region - write through
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
#endif
  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 6
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;

  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /* activate the OverDrive to reach the 200 Mhz Frequency */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
    Error_Handler();
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK) {
    Error_Handler();
  }

  TPI->ACPR = (HAL_RCC_GetSysClockFreq() / 2000000) - 1; // make the SWO working whatever settings in the system frequency - output is always 2MHz
}


void printConfig(void) {
  TPI->ACPR = (HAL_RCC_GetSysClockFreq() / 2000000) - 1; // make the SWO working whatever settings in the system frequency - output is always 2MHz

  // LCD initialization
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_SelectLayer(0);
  BSP_LCD_DisplayOff();
  LCD_LOG_Init();
  LCD_LOG_SetHeader((uint8_t *)"Instruction Cache example - larger code");
  printf("Running from address:   0x%08X \n", (uint32_t)main);
  printf("Data array1 at address: 0x%08X \n", (uint32_t)&array1);
  printf("Data array2 at address: 0x%08X \n", (uint32_t)&array2);
  printf("Compiler IAR, ver: %d \n", __VER__);
  printf("and Core is M7 | \n");
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void) {
  /* User may add here some code to deal with this error */
  while (1) {
  }
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void) {
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line) {
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1) {
  }
}
#endif
