/**
 ******************************************************************************
    @file    Template/Src/main.c
    @author  MCD Application Team
    @version V0.2.0
    @date    12-February-2015
    @brief   STM32F7xx HAL API Template project
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lcd_log.h"
#include "img1.h"
#include "img2.h"
#include "img3.h"
#include "img4.h"
#include "img5.h"
#include "img6.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NUM_IMAGES 4
#define FRAME_BUFFER_OFFSET (480*272*2)
#define MAX_ALPHA 128

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef     TimHandle;
DMA2D_HandleTypeDef   Dma2dHandle;
extern LTDC_HandleTypeDef  hLtdcHandler;
volatile uint32_t DMA2D_completed = 0;
volatile uint32_t frameBufferAddress;
volatile uint32_t frameBufferAddressChanged = 0;
volatile uint32_t time_start, time_end, time_diff;
// array of pointers to images
unsigned char const *images[] = {img6, img2, img3, img4};
uint8_t imagesAlpha[] = {0, 1 * 22, 2 * 22, 3 * 22 };
char resultText[50] = "init";

uint32_t i = 0;
/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void printConfig(void);
static void DMA2D_Config(void);
static void TransferComplete(DMA2D_HandleTypeDef *hdma2d);
static void TransferError(DMA2D_HandleTypeDef *hdma2d);
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d);
void TimeMeasureInit(void);
/* Private functions ---------------------------------------------------------*/

/**
    @brief  Main program
    @param  None
    @retval None
*/
int main(void) {
    /*  STM32F7xx HAL library initialization:
         - Configure the Flash prefetch, instruction and Data caches
         - Configure the Systick to generate an interrupt each 1 msec
         - Set NVIC Group Priority to 4
         - Low Level Initialization
    */
    HAL_Init();
    /* Configure the System clock to have a frequency of 200 MHz */
    SystemClock_Config();
    BSP_QSPI_Init();
    BSP_QSPI_MemoryMappedMode();
    TimeMeasureInit();
    // enable the caches for AXI interface
    SCB_EnableICache();
    SCB_EnableDCache();
    __HAL_FLASH_ART_ENABLE(); // enable the ART cache on ITCM flash
    BSP_LCD_Init();
    BSP_LCD_LayerDefaultInit(0, LCD_FB_START_ADDRESS);
    BSP_LCD_DisplayOn();
    frameBufferAddress = LCD_FB_START_ADDRESS + FRAME_BUFFER_OFFSET;//(uint32_t)testFrameBuffer;//LCD_FB_START_ADDRESS + FRAME_BUFFER_OFFSET;
    /* DMA2D configuration */
    DMA2D_Config();
    /* NVIC configuration for LTDC line interrupt */
    HAL_NVIC_SetPriority(LTDC_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(LTDC_IRQn);
    __HAL_LTDC_ENABLE_IT(&hLtdcHandler, LTDC_IT_RR);

    while (1) {
        i = 0;
        /* #### 1 - QPSI tranfer images to SDRAM frame buffer ### */
        TIM_MEASURE_START;

        while (i < NUM_IMAGES) { // show all images
            Dma2dHandle.LayerCfg[1].InputAlpha = imagesAlpha[i] < 128 ? imagesAlpha[i] : 256 - imagesAlpha[i]; // change slowly the alpha channel fo each image
            imagesAlpha[i] += 2;    // increment alpha channel of each image
            HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);

            if (HAL_DMA2D_BlendingStart_IT(&Dma2dHandle,
                                           (uint32_t)images[i], // Input image 480*272 of format RGB565 (16 bpp)
                                           frameBufferAddress,  // Output image of same size 480*272 after conversion by PFC in RGB565 (16 bpp)
                                           frameBufferAddress,
                                           LCD_X_SIZE,
                                           LCD_Y_SIZE) != HAL_OK) {
                /* Initialization Error */
                Error_Handler();
            }

            // wait till the tranfer is done
            while (DMA2D_completed == 0); // here the MCU is doing nothing - can do other tasks or go low power

            DMA2D_completed = 0;
            i++;  // select next image to transfer
        }

        TIM_MEASURE_END;
        /* #### 2 - Show the time measurement ### */
        hLtdcHandler.LayerCfg[0].FBStartAdress = frameBufferAddress;
        // show the line of time measurement
        sprintf(resultText,
                "Time in QSPI tranfers: %f ms, %f MB/s",
                (float)time_diff / (float)(HAL_RCC_GetSysClockFreq() / 1000),     // compute time in ms
                (float)LCD_X_SIZE * LCD_Y_SIZE * 2 * NUM_IMAGES / (float)(1024 * 1024) / ((float)time_diff / (float)(HAL_RCC_GetSysClockFreq())));
        BSP_LCD_DisplayStringAtLine(0, resultText);
        LTDC_LAYER(&hLtdcHandler, 0)->CFBAR = frameBufferAddress; // set the new layer frame buffer
        hLtdcHandler.Instance->SRCR = LTDC_SRCR_VBR;  // ask to change it after the end of current frame

        /* #### 3 - Implement double buffering ### */

        // wait till the end of current frame
        while (frameBufferAddressChanged == 0);  // here the MCU is doing nothing - can do other tasks or go low power

        frameBufferAddressChanged = 0;

        // make the double buffering
        if (frameBufferAddress == LCD_FB_START_ADDRESS) {
            frameBufferAddress = LCD_FB_START_ADDRESS + FRAME_BUFFER_OFFSET;
        } else {
            frameBufferAddress = LCD_FB_START_ADDRESS;
        }
    }
}


/**
    @brief DMA2D configuration.
    @note  This function Configure tha DMA2D peripheral :
           1) Configure the transfer mode : memory to memory with pixel format conversion (PFC)
           2) Configure the output color mode to ARGB4444
           3) Configure the output memory address at SRAM memory
           4) Configure the data size : 320x120 (pixels)
           5) Configure the input color mode as ARGB8888
           6) Configure the input memory address at FLASH memory
    @retval
     None
*/

static void DMA2D_Config(void) {
    /* Configure the DMA2D Mode, Color Mode and output offset */
    Dma2dHandle.Init.Mode         = DMA2D_M2M_BLEND; /* DMA2D mode Memory to Memory with Pixel Format Conversion */
    //Dma2dHandle.Init.ColorMode    = DMA2D_ARGB8888; /* DMA2D Output color mode is ARGB4444 (16 bpp) */
    Dma2dHandle.Init.ColorMode    = DMA2D_RGB565; /* DMA2D Output color mode is ARGB4444 (16 bpp) */
    Dma2dHandle.Init.OutputOffset = 0x0; /* No offset in output */
    /* DMA2D Callbacks Configuration */
    Dma2dHandle.XferCpltCallback  = TransferComplete;
    Dma2dHandle.XferErrorCallback = TransferError;
    /* Foreground layer Configuration : layer 1 */
    Dma2dHandle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    Dma2dHandle.LayerCfg[1].InputAlpha = 0xFF; /* Alpha fully opaque */
    Dma2dHandle.LayerCfg[1].InputColorMode = CM_RGB565; /* Layer 1 input format is ARGB8888 (32 bpp) */
    Dma2dHandle.LayerCfg[1].InputOffset = 0x0; /* No offset in input */
    /* Background layer Configuration */
    Dma2dHandle.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
    Dma2dHandle.LayerCfg[0].InputAlpha = 0xFF; /* 127 : semi-transparent */
// Dma2dHandle.LayerCfg[0].InputColorMode = CM_ARGB8888;
    Dma2dHandle.LayerCfg[0].InputColorMode = CM_RGB565;
    Dma2dHandle.LayerCfg[0].InputOffset = 0x0; /* No offset in input */
    Dma2dHandle.Instance = DMA2D;

    /* DMA2D Initialisation */
    if (HAL_DMA2D_Init(&Dma2dHandle) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

    if (HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

    if (HAL_DMA2D_ConfigLayer(&Dma2dHandle, 0) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }
}

/**
    @brief  DMA2D Transfer completed callback
    @param  hdma2d: DMA2D handle.
    @note   This example shows a simple way to report end of DMA2D transfer, and
            you can add your own implementation.
    @retval None
*/
static void TransferComplete(DMA2D_HandleTypeDef *hdma2d) {
    DMA2D_completed = 1;
}

/**
    @brief  DMA2D error callbacks
    @param  hdma2d: DMA2D handle
    @note   This example shows a simple way to report DMA2D transfer error, and you can
            add your own implementation.
    @retval None
*/
static void TransferError(DMA2D_HandleTypeDef *hdma2d) {
    Error_Handler();
}

/**
    @brief DMA2D MSP Initialization
           This function configures the hardware resources used in this example:
              - Peripheral's clock enable
              - Peripheral's GPIO Configuration
    @param hdma2d: DMA2D handle pointer
    @retval None
*/
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d) {
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    __HAL_RCC_DMA2D_CLK_ENABLE();
    /*##-2- NVIC configuration  ################################################*/
    /* NVIC configuration for DMA2D transfer complete interrupt */
    HAL_NVIC_SetPriority(DMA2D_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2D_IRQn);
}

static GPIO_InitTypeDef  GPIO_InitStruct;

void HAL_MspInit(void) {
    __HAL_RCC_GPIOI_CLK_ENABLE();
    GPIO_InitStruct.Pin = (GPIO_PIN_3);
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
}

/**
    @brief  System Clock Configuration
            The system Clock is configured as follow :
               System Clock source            = PLL (HSE)
               SYSCLK(Hz)                     = 200000000
               HCLK(Hz)                       = 200000000
               AHB Prescaler                  = 1
               APB1 Prescaler                 = 4
               APB2 Prescaler                 = 2
               HSE Frequency(Hz)              = 25000000
               PLL_M                          = 25
               PLL_N                          = 400
               PLL_P                          = 2
               PLL_Q                          = 8
               VDD(V)                         = 3.3
               Main regulator output voltage  = Scale1 mode
               Flash Latency(WS)              = 6
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
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 8;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    HAL_PWREx_EnableOverDrive();
    /*  Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
        clocks dividers */
    RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }
}

void TimeMeasureInit(void) {
    /* Set Timers instance */
    TimHandle.Instance = TIM2;
    __HAL_RCC_TIM2_CLK_ENABLE();
    __HAL_RCC_TIMCLKPRESCALER(RCC_TIMPRES_ACTIVATED); // run the timer on HCLK freq
    /*====================== Master configuration : TIM2 =======================*/
    /* Initialize TIM2 peripheral in counter mode*/
    TimHandle.Init.Period            = 0xFFFFFFFF;
    TimHandle.Init.Prescaler         = 0;
    TimHandle.Init.ClockDivision     = 0;
    TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TimHandle.Init.RepetitionCounter = 0;

    if (HAL_TIM_Base_Init(&TimHandle) != HAL_OK) {
        /* Initialization Error */
        Error_Handler();
    }

    HAL_TIM_Base_Start(&TimHandle);
}

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

#ifdef  USE_FULL_ASSERT

/**
    @brief  Reports the name of the source file and the source line number
            where the assert_param error has occurred.
    @param  file: pointer to the source file name
    @param  line: assert_param error line source number
    @retval None
*/
void assert_failed(uint8_t *file, uint32_t line) {
    /*  User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1) {
    }
}
#endif
