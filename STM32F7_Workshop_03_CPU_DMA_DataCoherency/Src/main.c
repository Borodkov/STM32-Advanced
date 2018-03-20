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
#include <string.h>

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define SRAM1_ADDRESS_START                        (0x20010000UL)
//#define SRAM1_ADDRESS_START                        (0x20008000UL)
#define SRAM1_SIZE                                 (0x17UL << 0UL)
#define SRAM1_REGION_NUMBER                        (0x00UL << MPU_RNR_REGION_Pos)
#define SRAM1_ACCESS_PERMISSION                    (0x03UL << MPU_RASR_AP_Pos)

#define __FORCE_WRITE_THROUGH()                   *(__IO uint32_t *)0xE000EF9C = 1UL<<2
/* Private variables ---------------------------------------------------------*/
/* DMA Handle declaration */
DMA_HandleTypeDef     DmaHandle;

static  uint32_t aSRC_Const_Buffer[BUFFER_SIZE] = {
  0x01020304, 0x05060708, 0x090A0B0C, 0x0D0E0F10,
  0x11121314, 0x15161718, 0x191A1B1C, 0x1D1E1F20,
  0x21222324, 0x25262728, 0x292A2B2C, 0x2D2E2F30,
  0x31323334, 0x35363738, 0x393A3B3C, 0x3D3E3F40,
  0x41424344, 0x45464748, 0x494A4B4C, 0x4D4E4F50,
  0x51525354, 0x55565758, 0x595A5B5C, 0x5D5E5F60,
  0x61626364, 0x65666768, 0x696A6B6C, 0x6D6E6F70,
  0x71727374, 0x75767778, 0x797A7B7C, 0x7D7E7F80
};

static uint32_t aDST_Buffer[BUFFER_SIZE];

static __IO uint32_t transferErrorDetected; /* Set to 1 if an error transfer is detected */
static __IO uint32_t compareErrorDetected = 0;
static __IO FlagStatus TransferCompleteFlag = RESET;

uint32_t *pBuffer = (uint32_t*)SRAM1_ADDRESS_START;

uint32_t timeS = 0;
uint32_t timeE = 0;
volatile uint32_t time[2] = {0, 0};

/* Private function prototypes -----------------------------------------------*/
static void DMA_Config(void);
static void MPU_Config(void);
void SystemClock_Config(void);
static void Error_Handler(void);
static void TransferComplete(DMA_HandleTypeDef *DmaHandle);
static void TransferError(DMA_HandleTypeDef *DmaHandle);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {
  /* STM32F7xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  uint32_t counter = 0;

  SCB_EnableICache();
  HAL_Init();

  /* Initialize LEDs */
  BSP_LED_Init(LED1);

  /* Configure the system clock to 200 MHz */
  SystemClock_Config();

  /* Set to 1 if an transfer error is detected */
  transferErrorDetected = 0;

  /* Fill 128 bytes with 0x55 pattern */
  memset((uint8_t*)SRAM1_ADDRESS_START, 0x55, sizeof(aSRC_Const_Buffer));

  /* Step-1 : Enable Data cache (uncomment following line to enable Data Cache */
  SCB_EnableICache();

  /* Step-2 : Perform cache clean operation by software (uncomment following line to
   * ensure coherency by software) */
  /* Clean Data Cache to update the content of the SRAM */
  SCB_CleanDCache_by_Addr(pBuffer, BUFFER_SIZE);

  /* Step-3 : Enable MPU and change SRAM region attribute (uncomment following line to
   * set write-through policy on SRAM) */
  MPU_Config();

  /* Step-4 : Force all CPU accesses to be write-through (uncomment following line to
   * set all CPU accesses to cachable region as write-through) */
  __FORCE_WRITE_THROUGH();

  for (counter = 0; counter < (sizeof(aSRC_Const_Buffer) / 4); counter++) {
    *pBuffer++ = aSRC_Const_Buffer[counter];
  }

  /* Configure and enable the DMA stream for Memory to Memory transfer */
  DMA_Config();

  /* Wait for DMA end-of-transfer */
  while (TransferCompleteFlag == RESET) {
  }

  /* Check data integrity*/
  pBuffer = (uint32_t*)&aDST_Buffer;

  for (counter = 0; counter < (sizeof(aSRC_Const_Buffer) / 4); counter++) {

    if (aSRC_Const_Buffer[counter] != *pBuffer) {
      compareErrorDetected++;
    }

    pBuffer++;
  }

  if (compareErrorDetected != 0) {
    compareErrorDetected = 0;

    /* Toggle LED1 fast */
    while (1) {
      BSP_LED_Toggle(LED1);
      HAL_Delay(100);
    }
  } else {
    /* Toggle LED1 slow */
    while (1) {
      BSP_LED_Toggle(LED1);
      HAL_Delay(500);
    }
  }
}

/**
  * @brief  Configure the DMA controller according to the Stream parameters
  *         defined in main.h file
  * @note  This function is used to :
  *        -1- Enable DMA2 clock
  *        -2- Select the DMA functional Parameters
  *        -3- Select the DMA instance to be used for the transfer
  *        -4- Select Callbacks functions called after Transfer complete and
               Transfer error interrupt detection
  *        -5- Initialize the DMA stream
  *        -6- Configure NVIC for DMA transfer complete/error interrupts
  *        -7- Start the DMA transfer using the interrupt mode
  * @param  None
  * @retval None
  */
static void DMA_Config(void) {
  /*## -1- Enable DMA2 clock #################################################*/
  __HAL_RCC_DMA2_CLK_ENABLE();

  /*##-2- Select the DMA functional Parameters ###############################*/
  DmaHandle.Init.Channel = DMA_CHANNEL;                     /* DMA_CHANNEL_0                    */
  DmaHandle.Init.Direction = DMA_MEMORY_TO_MEMORY;          /* M2M transfer mode                */
  DmaHandle.Init.PeriphInc = DMA_PINC_ENABLE;               /* Peripheral increment mode Enable */
  DmaHandle.Init.MemInc = DMA_MINC_ENABLE;                  /* Memory increment mode Enable     */
  DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD; /* Peripheral data alignment : Word */
  DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_WORD;    /* memory data alignment : Word     */
  DmaHandle.Init.Mode = DMA_NORMAL;                         /* Normal DMA mode                  */
  DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;              /* priority level : high            */
  DmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;           /* FIFO mode disabled               */
  DmaHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;
  DmaHandle.Init.MemBurst = DMA_MBURST_SINGLE;              /* Memory burst                     */
  DmaHandle.Init.PeriphBurst = DMA_PBURST_SINGLE;           /* Peripheral burst                 */

  /*##-3- Select the DMA instance to be used for the transfer : DMA2_Stream0 #*/
  DmaHandle.Instance = DMA_INSTANCE;

  /*##-4- Select Callbacks functions called after Transfer complete and Transfer error */
  DmaHandle.XferCpltCallback  = TransferComplete;
  DmaHandle.XferErrorCallback = TransferError;

  /*##-5- Initialize the DMA stream ##########################################*/
  if (HAL_DMA_Init(&DmaHandle) != HAL_OK) {
    /* Initialization Error */
    Error_Handler();
  }

  /*##-6- Configure NVIC for DMA transfer complete/error interrupts ##########*/
  /* Set Interrupt Group Priority */
  HAL_NVIC_SetPriority(DMA_INSTANCE_IRQ, 0, 0);

  /* Enable the DMA STREAM global Interrupt */
  HAL_NVIC_EnableIRQ(DMA_INSTANCE_IRQ);

  /*##-7- Start the DMA transfer using the interrupt mode ####################*/
  /* Configure the source, destination and buffer size DMA fields and Start DMA Stream transfer */
  /* Enable All the DMA interrupts */
  if (HAL_DMA_Start_IT(&DmaHandle, (uint32_t)SRAM1_ADDRESS_START, (uint32_t)aDST_Buffer, BUFFER_SIZE) != HAL_OK) {
    /* Transfer Error */
    Error_Handler();
  }
}

/**
  * @brief  Configures the MPU regions.
  * @param  None
  * @retval None
  */
static void MPU_Config(void) {
  /* Disable MPU */
  MPU->CTRL &= ~MPU_CTRL_ENABLE_Msk;

  /* Configure RAM region as Region N0, 256kB of size and R/W region */
  MPU->RNR  = SRAM1_REGION_NUMBER;
  MPU->RBAR = SRAM1_ADDRESS_START;

  /* Write-Back policy */
  MPU->RASR = SRAM1_SIZE  | MPU_RASR_C_Msk | SRAM1_ACCESS_PERMISSION ; //| 1<<MPU_RASR_TEX_Pos  | MPU_RASR_B_Msk

  /* Enable MPU */
  MPU->CTRL |= MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_ENABLE_Msk;
}

/**
  * @brief  DMA conversion complete callback
  * @note   This function is executed when the transfer complete interrupt
  *         is generated
  * @retval None
  */
static void TransferComplete(DMA_HandleTypeDef *DmaHandle) {
  TransferCompleteFlag = SET;
  /* Turn LED1 on: Transfer correct */
// BSP_LED_On(LED1);
}

/**
  * @brief  DMA conversion error callback
  * @note   This function is executed when the transfer error interrupt
  *         is generated during DMA transfer
  * @retval None
  */
static void TransferError(DMA_HandleTypeDef *DmaHandle) {
  transferErrorDetected = 1;
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
void SystemClock_Config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef ret = HAL_OK;

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);

  if (ret != HAL_OK) {
    while (1) { ; }
  }

  /* Activate the OverDrive to reach the 200 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();

  if (ret != HAL_OK) {
    while (1) { ; }
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);

  if (ret != HAL_OK) {
    while (1) { ; }
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void) {
  /* Turn LED4 on: Transfer Error */
// BSP_LED_On(LED4);
  while (1) {
  }
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line) {
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1) {
  }
}
#endif
