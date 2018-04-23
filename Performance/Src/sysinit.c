/**
 ******************************************************************************
    @file    stm32f7_performances/Src/sysinit.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This file contains the routine that initialize the system.
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "sysinit.h"
#include "stm32746g_discovery_lcd.h" /* LCD_FB_START_ADDRESS */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Cache_Init(void);
static void Error_Handler(void);
static void MPU_Config(void);

#if defined (DCACHE_ENABLE)
static void EnableDCache(void);
#endif

/* Private functions ---------------------------------------------------------*/
void system_init(void) {
    /* Initialize hardware layer */
    HAL_Init();
    /*  Disable ART beacause it was already enabled in HAL_Init(). Enable it if
        the flag ART_ENABLE has been activated */
    __HAL_FLASH_ART_DISABLE();
#if defined (ART_ENABLE)
    __HAL_FLASH_ART_ENABLE();
    __DSB();
#endif
#if defined (PF_ART_ENABLE)
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
#endif
    /* Configure the MPU for SRAM1, SRAM2 and LCD FrameBuffer in SDRAM */
    MPU_Config();
    /* Enable instruction or/and data caches */
    Cache_Init();
    /* Configure the System clock to have a frequency of 200 MHz */
    SystemClock_Config();
}

/**
    @brief  System Clock Configuration
            The system Clock is configured as follow :
               System Clock source            = PLL (HSE)
               SYSCLK(Hz)                     = 216000000
               HCLK(Hz)                       = 216000000
               AHB Prescaler                  = 1
               APB1 Prescaler                 = 4
               APB2 Prescaler                 = 2
               HSE Frequency(Hz)              = 25000000
               PLL_M                          = 25
               PLL_N                          = 432
               PLL_P                          = 2
               PLL_Q                          = 9
               VDD(V)                         = 3.3
               Main regulator output voltage  = Scale1 mode
               Flash Latency(WS)              = FLASH_WS: defined in the project configuration.
    @param  None
    @retval None
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
#ifdef DATA_IN_ExtSDRAM  /* FMC max frequency is 100MHz */
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLQ = 8;
#else
    RCC_OscInitStruct.PLL.PLLN = 432;
    RCC_OscInitStruct.PLL.PLLQ = 9;
#endif
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;

    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* activate the OverDrive to reach the 216 Mhz Frequency */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        Error_Handler();
    }

    /*  Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
        clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_WS) != HAL_OK) {
        Error_Handler();
    }
}

/**
    @brief  Enable instruction cache or/and data cache
    @param  None
    @retval None
*/
static void Cache_Init(void) {
#if defined (ICACHE_ENABLE)
    /* Enable instruction cache */
    SCB_EnableICache();
#endif /* ICACHE_ENABLE */
#if defined (DCACHE_ENABLE)
    /* Invalidate and enable data cache */
    EnableDCache();
#endif /* DCACHE_ENABLE */
}

/**
    @brief  Configure the MPU for SDRAM (LCD Frame buffer region only).
    @param  None
    @retval None
*/
static void MPU_Config(void) {
    MPU_Region_InitTypeDef MPU_InitStruct;
    /* Disable the MPU */
    HAL_MPU_Disable();
    /* Configure the MPU attributes as WT for LCD Frame buffer region */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = LCD_FB_START_ADDRESS;
    /*  Frame buffer size:
        - Pixel format used is: ARGB8888 -> the pixel is represented in 32-bit (8+8+8+8) -> 4 bytes
         Please refer to: BSP_LCD_LayerDefaultInit():
                          layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB8888;
        - The LCD size is 640x480.
        -> The frame buffer size = 640 x 480 x 4 = 1228800 bytes -> 1,2MByte.
          The nearest MPU size value is 2MB.
    */
    MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER1;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
    @brief  Reconfigure the systick timer for HAL
    @param  None
    @retval None
*/
void Systick_reconfig(void) {
    /*Configure the SysTick to have interrupt in 1ms time basis*/
    HAL_SYSTICK_Config(SystemCoreClock / 1000);
    /*Configure the SysTick IRQ priority */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0xF , 0);
}

#if defined (DCACHE_ENABLE)
/**
    @brief  Enable Data cache
    @param  None
    @retval None
*/
static void EnableDCache(void) {
#if (__DCACHE_PRESENT == 1U)
    uint32_t ccsidr;
    uint32_t sets;
    uint32_t ways;
    /* Level 1 data cache */
    SCB->CSSELR = (0U << 1U) | 0U;
    __DSB();
    ccsidr = SCB->CCSIDR;
    /* invalidate D-Cache */
    sets = (uint32_t)(CCSIDR_SETS(ccsidr));

    do {
        ways = (uint32_t)(CCSIDR_WAYS(ccsidr));

        do {
            SCB->DCISW = (((sets << SCB_DCISW_SET_Pos) & SCB_DCISW_SET_Msk) |
                          ((ways << SCB_DCISW_WAY_Pos) & SCB_DCISW_WAY_Msk));
            __DSB();
        } while (ways--);
    } while (sets--);

    __DSB();
    /* enable D-Cache */
    SCB->CCR |= (uint32_t)SCB_CCR_DC_Msk;
    __DSB();
    __ISB();
#endif
}
#endif


/**
    @brief  This function is executed in case of error occurrence.
    @param  None
    @retval None
*/
static void Error_Handler(void) {
    /* User may add here some code to deal with this error */
    while (1) {
    }
}
