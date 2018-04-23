/**
 ******************************************************************************
    @file    stm32f7_performances_DMAs/Src/system_init.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This file contains the routine that initializes the system.
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "system_init.h"
#include "stm32746g_discovery.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if (USE_DMA2 == 1)
/* DMA handler declaration */
DMA_HandleTypeDef   DmaHandle;
#endif

/* SPI handler declaration */
#if (USE_DMA1_SPI == 1)
SPI_HandleTypeDef SpiHandle;
#endif

/* Set to 1 if DMA2 transfer is correctly completed */
__IO uint8_t FlagDMA2TransferComplete = 0;
/* Set to 1 if DMA1 transfer is correctly completed */
__IO uint8_t FlagDMA1SPITransferComplete = 0;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Cache_Init(void);
static void MPU_Config(void);
static void Error_Handler(void);
#if defined (DCACHE_ENABLE)
static void EnableDCache(void);
static void CleanDCache(void);
#endif

#if (USE_DMA2 == 1)
static void DMA2_Transfer(uint32_t source_address, \
                          uint32_t destination_address, uint32_t buffer_size);
static void TransferCompleteDMA2(DMA_HandleTypeDef *DmaHandle);
static void TransferErrorDMA2(DMA_HandleTypeDef *DmaHandle);
#endif /* (USE_DMA2 == 1) */

#if (USE_DMA1_SPI == 1)
void DMA1_SPI_Tranceive(uint8_t *TxBuffer, uint8_t *RxBuffer, uint16_t size);
#endif /* (USE_DMA1_SPI == 1) */

/* Private functions ---------------------------------------------------------*/

/**
    @brief  Initialize the system
    @param  None
    @retval None
*/
void sys_init(void) {
#if (CHECK_TRANSFER == 1)
    RNG_HandleTypeDef rng_handle;
    __IO uint32_t counter = 0;
#endif
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
    /* Configure the MPU for SRAM1 nad DRAM2 */
    MPU_Config();
    /* Enable instruction or/and data caches */
    Cache_Init();
    /* Configure the System clock to have a frequency of 200 MHz */
    SystemClock_Config();
    /* Initialize LED_RED and LED_GREEN */
    BSP_LED_Init(LED1);
    /* Turn off LED_RED and LED_GREEN */
    BSP_LED_Off(LED1);
#if (CHECK_TRANSFER == 1)
    /* Configure the RNG peripheral to fill all RAMs with random values */
    rng_handle.Instance = RNG;

    if (HAL_RNG_DeInit(&rng_handle) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

    if (HAL_RNG_Init(&rng_handle) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

    __DSB();

    /* Fill the available region of the DTCMRAM used by the DMAs with random values */
    for (counter = 0; counter < DTCMRAM_DMA_AVAILABLE_SIZE; counter++) {
        *(uint8_t *)(DTCMRAM_DMA_START_ADDRESS + counter) = (uint8_t)HAL_RNG_GetRandomNumber(&rng_handle);
    }

    __DSB();

    /* Fill the available region of the SRAM1 used by the DMAs with random values */
    for (counter = 0; counter < SRAM1_DMA_AVAILABLE_SIZE; counter++) {
        *(uint8_t *)(SRAM1_DMA_START_ADDRESS + counter) = (uint8_t)HAL_RNG_GetRandomNumber(&rng_handle);
    }

    __DSB();

    /* Fill the available region of the SRAM2 used by the DMAs with random values */
    for (counter = 0; counter < SRAM2_DMA_AVAILABLE_SIZE; counter++) {
        *(uint8_t *)(SRAM2_DMA_START_ADDRESS + counter) = (uint8_t)HAL_RNG_GetRandomNumber(&rng_handle);
    }

    __DSB();
#if defined (DATA_IN_ExtSDRAM)

    /* Fill the available region of the SDRAM used by the DMAs with random values */
    for (counter = 0; counter < SDRAM_DMA_AVAILABLE_SIZE; counter++) {
        *(uint8_t *)(SDRAM_DMA_START_ADDRESS + counter) = (uint8_t)HAL_RNG_GetRandomNumber(&rng_handle);
    }

    __DSB();
#endif /* DATA_IN_ExtSDRAM */
#endif /* (CHECK_TRANSFER == 1) */
#if defined (DCACHE_ENABLE)
    /* Clean D-Cache */
    CleanDCache();
#endif
#if (USE_DMA1_SPI)
    /* Send and receive data to/from SPI3 using DMA1 */
    DMA1_SPI_Tranceive((uint8_t *)((uint32_t *)DMA1SPI_SRC_ADDRESS), \
                       (uint8_t *)((uint32_t *)DMA1SPI_DST_ADDRESS), DMA1SPI_TRANSFER_SIZE);
#endif
#if (USE_DMA2 == 1)
    /* Transfer data form MEM A to MEM B using DMA2 */
    DMA2_Transfer(DMA2_SRC_ADDRESS, DMA2_DST_ADDRESS, DMA2_TRANSFER_SIZE);
#endif
}

/**
    @brief  Configure the MPU for SRAM1, DRAM2 and SDRAM.
    @param  None
    @retval None
*/
static void MPU_Config(void) {
    MPU_Region_InitTypeDef MPU_InitStruct;
    /* Disable the MPU */
    HAL_MPU_Disable();
    /* Configure the MPU attributes as WT for SRAM1 and SRAM2 */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.BaseAddress = 0x20010000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER1;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
    MPU_InitStruct.SubRegionDisable = 0x00;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    /* Configure the MPU attributes as WT for SDRAM (only DMA working region) */
    MPU_InitStruct.BaseAddress = SDRAM_DMA_START_ADDRESS;
    MPU_InitStruct.Size = MPU_REGION_SIZE_64KB;
    MPU_InitStruct.Number = MPU_REGION_NUMBER2;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    /* Configure the MPU attributes as WT for SDRAM For frame buffer */
    MPU_InitStruct.BaseAddress = LCD_FB_START_ADDRESS;
    MPU_InitStruct.Size = MPU_REGION_SIZE_2MB;
    MPU_InitStruct.Number = MPU_REGION_NUMBER3;
    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    /* Enable the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
    __DSB();
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

/**
    @brief  Clean Data cache
    @param  None
    @retval None
*/
static void CleanDCache(void) {
#if (__DCACHE_PRESENT == 1U)
    uint32_t ccsidr;
    uint32_t sets;
    uint32_t ways;
    SCB->CSSELR = (0U << 1U) | 0U;          /* Level 1 data cache */
    __DSB();
    ccsidr = SCB->CCSIDR;
    /* clean D-Cache */
    sets = (uint32_t)(CCSIDR_SETS(ccsidr));

    do {
        ways = (uint32_t)(CCSIDR_WAYS(ccsidr));

        do {
            SCB->DCCSW = (((sets << SCB_DCCSW_SET_Pos) & SCB_DCCSW_SET_Msk) |
                          ((ways << SCB_DCCSW_WAY_Pos) & SCB_DCCSW_WAY_Msk));
            __DSB();
        } while (ways--);
    } while (sets--);

    __DSB();
    __ISB();
#endif
}
#endif

/**
    @brief  Configure the system clock
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

    /* Activate the OverDrive to reach the 216 Mhz Frequency */
    if (HAL_PWREx_EnableOverDrive() != HAL_OK) {
        Error_Handler();
    }

    /*  Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
        clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK \
                                   | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource =  RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_WS) != HAL_OK) {
        Error_Handler();
    }
}

#if (USE_DMA2 == 1)
/**
    @brief  Transfer data from memory A to Memory B by means of DMA2
    @param  source_address: dma2 memory source address
            destination_address: dma2 memory destination address
            buffer_size: the size of the transfer in byte
    @retval None
*/
void DMA2_Transfer(uint32_t source_address, uint32_t destination_address, uint32_t buffer_size) {
    /* Enable DMA2 clock  */
    __HAL_RCC_DMA2_CLK_ENABLE();
    __DSB();
    /* Select the DMA2 functional Parameters */
    DmaHandle.Init.Channel = DMA2_CHANNEL;                    /* DMA_CHANNEL                      */
    DmaHandle.Init.Direction = DMA_MEMORY_TO_MEMORY;          /* M2M transfer mode                */
    DmaHandle.Init.PeriphInc = DMA_PINC_ENABLE;               /* Peripheral increment mode Enable */
    DmaHandle.Init.MemInc = DMA_MINC_ENABLE;                  /* Memory increment mode Enable     */
    DmaHandle.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE; /* Peripheral data alignment : Byte */
    DmaHandle.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;    /* memory data alignment : Byte     */
    DmaHandle.Init.Mode = DMA_NORMAL;                         /* Normal DMA mode                  */
    DmaHandle.Init.Priority = DMA_PRIORITY_HIGH;              /* priority level : high            */
    DmaHandle.Init.FIFOMode = DMA_FIFOMODE_DISABLE;           /* FIFO mode disabled               */
    DmaHandle.Init.FIFOThreshold = DMA_FIFO_THRESHOLD_FULL;   /* FIFO threshold: FULL             */
    DmaHandle.Init.MemBurst = DMA_MBURST_SINGLE;              /* Memory burst                     */
    DmaHandle.Init.PeriphBurst = DMA_PBURST_SINGLE;           /* Peripheral burst                 */
    /* Select the DMA2 instance to be used for the transfer */
    DmaHandle.Instance = DMA2_INSTANCE;
    /* Select Callbacks functions called after Transfer complete and Transfer error */
    DmaHandle.XferCpltCallback  = TransferCompleteDMA2;
    DmaHandle.XferErrorCallback = TransferErrorDMA2;

    /* Initialize the DMA2 stream  */
    if (HAL_DMA_Init(&DmaHandle) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

#ifdef CHECK_TRANSFER_DMA2
    /* Configure the NVIC for DMA2 transfer complete/error interrupts */
    /* Set Interrupt Group Priority */
    HAL_NVIC_SetPriority(DMA2_INSTANCE_IRQ, 0, 0);
    /* Enable the DMA2 STREAM global Interrupt */
    HAL_NVIC_EnableIRQ(DMA2_INSTANCE_IRQ);
#endif

    /* Start the DMA2 transfer using the interrupt mode */
    /*  Configure the source, destination and buffer size DMA2 fields and
        Start DMA Stream transfer */
    /* Enable All the DMA interrupts */
    if (HAL_DMA_Start_IT((DMA_HandleTypeDef *)&DmaHandle, source_address, \
                         destination_address, buffer_size) != HAL_OK) {
        /* Transfer Error */
        Error_Handler();
    }
}

/**
    @brief  DMA2 transfer complete callback
    @note   This function is executed when the DMA2 transfer complete interrupt
            is generated
    @retval None
*/
void TransferCompleteDMA2(DMA_HandleTypeDef *DmaHandle) {
    FlagDMA2TransferComplete = FLAG_END_DMA2_TRANSFER ;
}

/**
    @brief  DMA2 transfer error callback
    @note   This function is executed when the transfer error interrupt
            is generated during DMA2 transfer
    @retval None
*/
void TransferErrorDMA2(DMA_HandleTypeDef *DmaHandle) {
    BSP_LED_On(LED_RED);
}

#endif /* USE_DMA2 == 1 */


#if (USE_DMA1_SPI == 1)
/**
    @brief  Transfer data from memory A to Memory B by means of DMA1 and
            SPI3 which send/receives data in full duplex mode.
    @param  TxBuffer: a pointer to the buffer to be sent
            RxBuffer: a pointer to the buffer to be received
            size: the size of the transfer in byte.
    @retval None
*/
void DMA1_SPI_Tranceive(uint8_t *TxBuffer, uint8_t *RxBuffer, uint16_t size) {
    /* Configure the SPI peripheral */
    /* Set the SPI parameters */
    SpiHandle.Instance               = SPIx;
    SpiHandle.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2; /* 25Mb/s @216MHz */
    SpiHandle.Init.Direction         = SPI_DIRECTION_2LINES;
    SpiHandle.Init.CLKPhase          = SPI_PHASE_1EDGE;
    SpiHandle.Init.CLKPolarity       = SPI_POLARITY_HIGH;
    SpiHandle.Init.CRCCalculation    = SPI_CRCCALCULATION_DISABLE;
    SpiHandle.Init.CRCPolynomial     = 7;
    SpiHandle.Init.DataSize          = SPI_DATASIZE_8BIT;
    SpiHandle.Init.FirstBit          = SPI_FIRSTBIT_MSB;
    SpiHandle.Init.NSS               = SPI_NSS_SOFT;
    SpiHandle.Init.TIMode            = SPI_TIMODE_DISABLE;
    SpiHandle.Init.Mode = SPI_MODE_MASTER;

    /* De-initialize the SPI */
    if (HAL_SPI_DeInit(&SpiHandle) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

    /* Initialize the SPI */
    if (HAL_SPI_Init(&SpiHandle) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

    /* Start the Full Duplex Communication process */
    if (HAL_SPI_TransmitReceive_DMA(&SpiHandle, TxBuffer, RxBuffer, size) != HAL_OK) {
        /* Transfer error in transmission process */
        Error_Handler();
    }
}
/**
    @brief  TxRx Transfer completed callback.
    @param  hspi: SPI handle.
    @retval None
*/
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    FlagDMA1SPITransferComplete = FLAG_END_DMA1_TRANSFER;
}

/**
    @brief  SPI error callback.
    @param  hspi: SPI handle
    @retval None
*/
void HAL_SPI_ErrorCallback(SPI_HandleTypeDef *hspi) {
    /* Turn LED_RED on: Transfer error in SPI reception/transmission */
    BSP_LED_On(LED_RED);
}
#endif /* USE_DMA1_SPI */


/**
    @brief  This function is executed in case of error occurrence.
    @param  None
    @retval None
*/
static void Error_Handler(void) {
    while (1) {
    }
}
