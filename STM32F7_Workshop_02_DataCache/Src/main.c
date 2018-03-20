/**
  ******************************************************************************
  * File Name          : main.c
  * Description        : Main program body
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_sdram.h"
#include  "lcd_log.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NUM_SAMPLES 500
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint32_t result[4];
uint32_t array_sdram[NUM_SAMPLES] @ 0xC0000000;   // in external SDRAM
uint32_t array_dtcm[NUM_SAMPLES];  // in internal DTCM SRAM
uint32_t time_start, time_end;
volatile uint32_t time_diff;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void MPU_Config(void);
static void CPU_CACHE_Enable(void);
static void printConfig(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {
  uint32_t n, acc;

  HAL_Init();

  SystemClock_Config();

  BSP_SDRAM_Init();

  // initialize the array with some values
  for (n = 0; n < NUM_SAMPLES; n++) {
    array_sdram[n] = n;
    array_dtcm[n] = n;
  }

  result[0] = result[1] = result[2] = result[3] = 0;

  MPU_Config(); // configure the memory region
  printConfig();
  printf("->CPU cycles spent for sum of %d values \n", NUM_SAMPLES);
  HAL_SYSTICK_Config(0xFFFFFF);

  acc = 0;
  // DTCM data
  TIM_MEASURE_START;

  for (n = 0; n < NUM_SAMPLES; n++) {
    acc += (array_dtcm[n]);
  }

  TIM_MEASURE_END;
  result[0] = acc;   // read out the accumulator into volatile variable in order to not loose the content
  printf(" DTCM data (array_dtcm):                   %d cycles\n", time_diff);

  acc = 0;
  // No D-Cache - external memory
  TIM_MEASURE_START;

  for (n = 0; n < NUM_SAMPLES; n++) {
    acc += (array_sdram[n]);
  }

  TIM_MEASURE_END;
  result[1] = acc;   // read out the accumulator into volatile variable in order to not loose the content
  printf(" D-Cache disabled (array_sdram):            %d cycles\n", time_diff);

  // enable the D-Cache now
  SCB_EnableDCache();

  acc = 0;
  // D-Cache Enabled - first run
  TIM_MEASURE_START;

  for (n = 0; n < NUM_SAMPLES; n++) {
    acc += (array_sdram[n]);
  }

  TIM_MEASURE_END;
  result[2] = acc;   // read out the accumulator into volatile variable in order to not loose the content
  printf(" D-Cache enabled, first run (array_sdram):  %d cycles\n", time_diff);

  acc = 0;
  // D-Cache Enabled - second run
  TIM_MEASURE_START;

  for (n = 0; n < NUM_SAMPLES; n++) {
    acc += (array_sdram[n]);
  }

  TIM_MEASURE_END;
  result[3] = acc;   // read out the accumulator into volatile variable in order to not loose the content
  printf(" D-Cache enabled, second run (array_sdram): %d cycles\n", time_diff);

  printf(" results of computation are : %d,%d,%d,%d\n", result[0], result[1], result[2], result[3]);
  BSP_LCD_DisplayOn();

  while (1);

}


/**
  * @brief  Configure the MPU attributes as Write Through for SRAM1/2.
  * @note   The Base Address is 0x20010000 since this memory interface is the AXI.
  *         The Region Size is 256KB, it is related to SRAM1 and SRAM2  memory size.
  * @param  None
  * @retval None
  */
#ifdef ARM_MATH_CM7
static void MPU_Config(void) {
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0xC0000000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_128KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

#define DCACHE_WBWA

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
#endif

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
}


void printConfig(void) {
  TPI->ACPR = (HAL_RCC_GetSysClockFreq() / 2000000) - 1; // make the SWO working whatever settings in the system frequency - output is always 2MHz

#ifdef ARM_MATH_CM7
  // LCD initialization
  BSP_LCD_Init();
  BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
  BSP_LCD_SelectLayer(0);
  BSP_LCD_DisplayOff();
  LCD_LOG_Init();
  LCD_LOG_SetHeader((uint8_t *)"Data Cache example");
#endif
  printf("WS: %2d, ART: %s, D_cache: %s, I_cache: %s\n", FLASH->ACR & 0xF, (FLASH->ACR & 0x200) ? "ON " : "OFF", ((SCB->CCR & 0x10000) ? "ON " : "OFF"), ((SCB->CCR & 0x20000) ? "ON " : "OFF"));
  printf("System Clock: %9d, cycles: %d\n", HAL_RCC_GetSysClockFreq(), time_diff);
  printf("Running from address:   0x%08X \n", (uint32_t)main);
  printf("Data array_sdram at address: 0x%08X \n", (uint32_t)&array_sdram);
  printf("Data array_dtcm at address: 0x%08X \n", (uint32_t)&array_dtcm);
  printf("Compiler IAR, ver: %d \n\r", __VER__);
  printf("and Core is M7 | \n\r");
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
