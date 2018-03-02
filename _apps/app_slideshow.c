/**
 ******************************************************************************
 * @file    QSPI/QSPI_perfs/Src/main.c
 * @author  MCD Application Team
 * @brief   QSPI performance application
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "stm32746g_discovery.h"
#include "lcd_log.h"

#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_sdram.h"
#include "stm32746g_discovery_qspi.h"

#include "img1.h"
#include "img2.h"
#include "img3.h"
#include "img4.h"
#include "img5.h"
#include "img6.h"
#include "icon_S.h"
#include "icon_T.h"
#include "icon_M.h"

/* Global variables ---------------------------------------------------------*/
DMA2D_HandleTypeDef   Dma2dHandle;
TIM_HandleTypeDef     TimHandle;

/* Private typedef -----------------------------------------------------------*/
#define LCD_X_SIZE  RK043FN48H_WIDTH    /* RK043FN48H LCD used on discovery board */
#define LCD_Y_SIZE  RK043FN48H_HEIGHT

#define NUM_IMAGES  6
#define NUM_ICONS   3
#define FRAME_BUFFER_OFFSET (LCD_X_SIZE * LCD_Y_SIZE * 2)
#define MAX_ALPHA 128

#define ICON_MIN_X  (-100)
#define ICON_MIN_Y  (-100)
#define ICON_MAX_X  (LCD_X_SIZE + 100)
#define ICON_MAX_Y  (LCD_Y_SIZE + 100)

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define TIM_MEASURE_START time_start = __HAL_TIM_GET_COUNTER(&TimHandle);

#define TIM_MEASURE_END time_end = __HAL_TIM_GET_COUNTER(&TimHandle); \
                        time_diff = time_end - time_start;

/* Private variables ---------------------------------------------------------*/
static osSemaphoreId xBinSem_DMA2DCompletedHandle = NULL;

/* Private function prototypes -----------------------------------------------*/
static void DMA2D_Config(int32_t x_size, int32_t x_size_orig, uint32_t ColorMode, uint8_t alpha);
static void TransferComplete(DMA2D_HandleTypeDef *hdma2d);
static void TransferError(DMA2D_HandleTypeDef *hdma2d);
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d);
void TimeMeasureInit(void);
static uint32_t  myClip(int32_t * x0, int32_t * y0, int32_t * xsize, int32_t * ysize, const uint8_t ** pPixel, int pitch, int bytesPerPixel);
static void FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex);

/**
  * @brief
  * @param
  * @retval None
  */
static void task(void const *pvParameters) {
  /* array of pointers to images */
  unsigned char const *images[] = {img1, img2, img3, img4, img5, img6};
  unsigned char const *icons[] = {icon_S, icon_T, icon_M};
  int32_t icons_x[] = {50, 160, 280};
  int32_t icons_y[] = {10, 50, 100};
  uint32_t icons_size_x[] = {icon_S_width, icon_T_width, icon_M_width};
  uint32_t icons_size_y[] = {icon_S_height, icon_T_height, icon_M_height};
  uint8_t imagesAlpha[] = {0, 1 * 22, 2 * 22, 3 * 22 };
  uint32_t i, dataTransferred;
  int32_t img_x, img_y, img_size_x, img_size_y, img_size_x_orig;
  uint8_t *img_pData;
  BSP_QSPI_Init();
  BSP_QSPI_MemoryMappedMode();
  /* Configure QSPI: LPTR register with the low-power time out value */
  WRITE_REG(QUADSPI->LPTR, 0xFFF);
  TimeMeasureInit();
  /* LCD initialization in RGB565 format */
  BSP_LCD_Init();
  BSP_LCD_LayerRgb565Init(0, LCD_FB_START_ADDRESS);
  BSP_LCD_SetFont(&Font12);
  BSP_LCD_DisplayOn();
  uint32_t frameBufferAddress = LCD_FB_START_ADDRESS + FRAME_BUFFER_OFFSET;

  /* Infinite loop */
  for (;;) {
    dataTransferred = 0;
    /* #### 1 - QPSI transfer images to SDRAM frame buffer ### */
    /* Clear frame buffer using DMA2D */
    FillBuffer(0, (uint32_t *)(frameBufferAddress), LCD_X_SIZE, LCD_Y_SIZE, 0, LCD_LOG_BACKGROUND_COLOR);
    uint32_t time_start, time_end, time_diff;
    TIM_MEASURE_START;
    i = 0;

    while (i < NUM_IMAGES) { /* show all images */
      DMA2D_Config(LCD_X_SIZE, LCD_X_SIZE, CM_RGB565, imagesAlpha[i] < 128 ? imagesAlpha[i] : 256 - imagesAlpha[i]);
      imagesAlpha[i] += 2;    /* increment alpha channel of each image */
      HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);
      HAL_DMA2D_BlendingStart_IT(&Dma2dHandle,
                                 (uint32_t)images[i], /* Input image 480*272 of format RGB565 (16 bpp) */
                                 frameBufferAddress,  /* Output image 480*272 after conversion by PFC in ARGB888 (32 bpp) */
                                 frameBufferAddress,
                                 LCD_X_SIZE,
                                 LCD_Y_SIZE);
      /* wait till the transfer is done */
      /* here the MCU is doing nothing - can do other tasks or go low power */
      osSemaphoreWait(xBinSem_DMA2DCompletedHandle, osWaitForever); // w8 semaphore
      dataTransferred += LCD_X_SIZE * LCD_Y_SIZE * 2;
      ++i;  /* select next image to transfer */
    }

    i = 0;

    while (i < NUM_ICONS) { /* show all letters */
      img_x = icons_x[i];
      img_y = icons_y[i];
      img_size_x = img_size_x_orig = icons_size_x[i];
      img_size_y = icons_size_y[i];;
      img_pData = (uint8_t*)icons[i];

      if (0 == myClip(&img_x, &img_y, &img_size_x, &img_size_y, (const uint8_t **)&img_pData, img_size_x, 2)) {
        DMA2D_Config(img_size_x, img_size_x_orig, CM_ARGB4444, 0xFF);
        HAL_DMA2D_BlendingStart_IT(&Dma2dHandle,
                                   (uint32_t)img_pData, /* Input image in format ARGB444 (16 bpp) */
                                   frameBufferAddress + (img_x * 2) + (img_y * LCD_X_SIZE * 2), /* Output image after conversion by PFC in RGB565 (16 bpp) */
                                   frameBufferAddress + (img_x * 2) + (img_y * LCD_X_SIZE * 2),
                                   img_size_x,
                                   img_size_y) ;
        /* wait till the transfer is done */
        /* here the MCU is doing nothing - can do other tasks or go low power */
        osSemaphoreWait(xBinSem_DMA2DCompletedHandle, osWaitForever); // w8 semaphore
        dataTransferred += img_size_x * img_size_y * 2;
      }

      icons_y[0] += 1;
      icons_y[1] -= 2;
      icons_y[2] += 4;

      if (icons_x[i] > ICON_MAX_X) icons_x[i] = ICON_MIN_X;

      if (icons_x[i] < ICON_MIN_X) icons_x[i] = ICON_MAX_X;

      if (icons_y[i] > ICON_MAX_Y) icons_y[i] = ICON_MIN_Y;

      if (icons_y[i] < ICON_MIN_Y) icons_y[i] = ICON_MAX_Y;

      ++i;  /* select next image to transfer */
    }

    TIM_MEASURE_END;
    /* Change LCD buffer address */
    BSP_LCD_SetLayerAddress(0, frameBufferAddress);
    /* #### 2 - Show the time measurement ### */
    /* show the line of time measurement */
    char resultText[50];
    sprintf(resultText,
            "Time in QSPI transfers: %f ms, %f MB/s",
            (float)time_diff / (float)(HAL_RCC_GetSysClockFreq() / 1000), /* compute time in ms **/
            (float)(dataTransferred) / (float)(1024 * 1024) / ((float)time_diff / (float)(HAL_RCC_GetSysClockFreq())));
    BSP_LCD_DisplayStringAtLine(0, (uint8_t*)resultText);
    /* Wait for 15 ms */
    osDelay(15);

    /* Use double buffering mechanism */
    if (frameBufferAddress == LCD_FB_START_ADDRESS)
      frameBufferAddress = LCD_FB_START_ADDRESS + FRAME_BUFFER_OFFSET;
    else
      frameBufferAddress = LCD_FB_START_ADDRESS;
  }
}

/*----------------------------------------------------------------------------*/
void vStartSlideShowTask() {
//  LCD_UsrLog("vStartSlideShowTask()\n");
  /* definition and creation of xBinSem_... */
  osSemaphoreDef(xBinSem_DMA2DCompleted);
  xBinSem_DMA2DCompletedHandle = osSemaphoreCreate(osSemaphore(xBinSem_DMA2DCompleted), 1);
  /* take semaphore ... */
  osSemaphoreWait(xBinSem_DMA2DCompletedHandle, 0);
  /* Create that task */
  osThreadDef(SHOW,
              task,
              osPriorityNormal,
              0,
              0x1000);
  osThreadCreate(osThread(SHOW), NULL);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  DMA2D configuration.
  * @note   This function Configure tha DMA2D peripheral :
  * @retval None
  */
static void DMA2D_Config(int32_t x_size, int32_t x_size_orig, uint32_t ColorMode, uint8_t alpha) {
  /* Configure the DMA2D Mode, Color Mode and output offset */
  Dma2dHandle.Init.Mode         = DMA2D_M2M_BLEND; /* DMA2D mode Memory to Memory with Pixel Format Conversion */
  Dma2dHandle.Init.ColorMode    = DMA2D_RGB565; /* DMA2D Output color mode is RGB565 (16 bpp) */
  Dma2dHandle.Init.OutputOffset = (LCD_X_SIZE - x_size) ; /* No offset in output */
  /* DMA2D Callbacks Configuration */
  Dma2dHandle.XferCpltCallback  = TransferComplete;
  Dma2dHandle.XferErrorCallback = TransferError;
  /* Foreground layer Configuration : layer 1 */
  Dma2dHandle.LayerCfg[1].AlphaMode = DMA2D_COMBINE_ALPHA;
  Dma2dHandle.LayerCfg[1].InputAlpha = alpha; /* Alpha fully opaque */
  Dma2dHandle.LayerCfg[1].InputColorMode = ColorMode; /* Layer 1 input format */
  Dma2dHandle.LayerCfg[1].InputOffset = x_size_orig - x_size ; /* No offset in input */
  /* Background layer Configuration */
  Dma2dHandle.LayerCfg[0].AlphaMode = DMA2D_REPLACE_ALPHA;
  Dma2dHandle.LayerCfg[0].InputAlpha = 0xFF; /* 127 : semi-transparent */
  Dma2dHandle.LayerCfg[0].InputColorMode = CM_RGB565;
  Dma2dHandle.LayerCfg[0].InputOffset = (LCD_X_SIZE - x_size) ; /* No offset in input */
  Dma2dHandle.Instance = DMA2D;
  /* DMA2D Initialization */
  HAL_DMA2D_Init(&Dma2dHandle);
  HAL_DMA2D_ConfigLayer(&Dma2dHandle, 1);
  HAL_DMA2D_ConfigLayer(&Dma2dHandle, 0);
}

/**
  * @brief  DMA2D Transfer completed callback
  * @param  hdma2d: DMA2D handle.
  * @note   This example shows a simple way to report end of DMA2D transfer
  * @retval None
  */
static void TransferComplete(DMA2D_HandleTypeDef *hdma2d) {
  osSemaphoreRelease(xBinSem_DMA2DCompletedHandle);
}

/**
  * @brief  DMA2D error callbacks
  * @param  hdma2d: DMA2D handle
  * @note   This example shows a simple way to report DMA2D transfer error,
  * @retval None
  */
static void TransferError(DMA2D_HandleTypeDef *hdma2d) {
  //Error_Handler();
  while (1)
    ;
}

static uint32_t myClip(int32_t * x0, int32_t * y0, int32_t * xsize, int32_t * ysize, const uint8_t ** pPixel, int pitch, int bytesPerPixel) {
  int clip_x0, clip_x1, clip_y0, clip_y1;
  /* get the actual clipping rectangle */
  clip_x0 = 0;
  clip_y0 = 0;
  clip_x1 = LCD_X_SIZE - 1;
  clip_y1 = LCD_Y_SIZE - 1;

  /* check if clipping needs to be done on all dimensions */
  if (clip_x0 > *x0) {
    *xsize -= clip_x0 - *x0; /* shrink the width */
    *pPixel += bytesPerPixel * (clip_x0 - *x0); /* update the pixel pointer */
    *x0 = clip_x0; /* mark the new start of line */
  }

  if (clip_y0 > *y0) {
    *ysize -= clip_y0 - *y0; /* shrink the height */
    *pPixel += bytesPerPixel * pitch * (clip_y0 - *y0); /* update the pixel pointer */
    *y0 = clip_y0; /* mark the new line */
  }

  if (clip_y1 < *y0 + *ysize) {
    *ysize -= *y0 + *ysize - clip_y1; /* shrink the height */
  }

  if (clip_x1 < *x0 + *xsize) {
    *xsize -= *x0 + *xsize - clip_x1; /* shrink the height */
  }

  if (clip_x1 - clip_x0 + 1 < *xsize)
    *xsize = clip_x1 - clip_x0 + 1; /* shrink the width from right side */

  if (clip_y1 - clip_y0 + 1 < *ysize)
    *ysize = clip_y1 - clip_y0 + 1; /* shrink the height from bottom */

  if (*xsize <= 0 || *ysize <= 0)
    return 1;

  return 0;
}

/**
  * @brief DMA2D MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration
  * @param hdma2d: DMA2D handle pointer
  * @retval None
  */
void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d) {
  /*##-1- Enable peripherals and GPIO Clocks #################################*/
  __HAL_RCC_DMA2D_CLK_ENABLE();
  /*##-2- NVIC configuration  ################################################*/
  /* NVIC configuration for DMA2D transfer complete interrupt */
  HAL_NVIC_SetPriority(DMA2D_IRQn, 6, 0);
  HAL_NVIC_EnableIRQ(DMA2D_IRQn);
}

// void HAL_MspInit(void)
// {
//   __HAL_RCC_GPIOI_CLK_ENABLE();

//   GPIO_InitTypeDef  GPIO_InitStruct;
//   GPIO_InitStruct.Pin = (GPIO_PIN_3);
//   GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
//   GPIO_InitStruct.Pull = GPIO_PULLUP;
//   GPIO_InitStruct.Speed = GPIO_SPEED_FAST;

//   HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
// }

void TimeMeasureInit(void) {
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_TIMCLKPRESCALER(RCC_TIMPRES_ACTIVATED); /* run the timer on HCLK freq */
  /* Initialize TIM2 peripheral in counter mode*/
  TimHandle.Instance               = TIM2;
  TimHandle.Init.Period            = 0xFFFFFFFF;
  TimHandle.Init.Prescaler         = 0;
  TimHandle.Init.ClockDivision     = 0;
  TimHandle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  TimHandle.Init.RepetitionCounter = 0;
  HAL_TIM_Base_Init(&TimHandle);
  HAL_TIM_Base_Start(&TimHandle);
}

/**
  * @brief  Fills a buffer using DMA2D.
  * @param  LayerIndex: Layer index
  * @param  pDst: Pointer to destination buffer
  * @param  xSize: Buffer width
  * @param  ySize: Buffer height
  * @param  OffLine: Offset
  * @param  ColorIndex: Color index
  * @retval None
  */
static void FillBuffer(uint32_t LayerIndex, void *pDst, uint32_t xSize, uint32_t ySize, uint32_t OffLine, uint32_t ColorIndex) {
  DMA2D_HandleTypeDef hDma2dHandler;
  hDma2dHandler.Instance = DMA2D;
  /* Register to memory mode with RGB565 as color Mode */
  hDma2dHandler.Init.Mode         = DMA2D_R2M;
  hDma2dHandler.Init.ColorMode    = DMA2D_RGB565;
  hDma2dHandler.Init.OutputOffset = OffLine;
  /* Layer Configuration */
  hDma2dHandler.LayerCfg[LayerIndex].AlphaMode = DMA2D_NO_MODIF_ALPHA;
  hDma2dHandler.LayerCfg[LayerIndex].InputAlpha = 0xFF;
  hDma2dHandler.LayerCfg[LayerIndex].InputColorMode = DMA2D_RGB565;
  hDma2dHandler.LayerCfg[LayerIndex].InputOffset = 0;

  /* DMA2D Initialization */
  if (HAL_DMA2D_Init(&hDma2dHandler) == HAL_OK) {
    if (HAL_DMA2D_ConfigLayer(&hDma2dHandler, LayerIndex) == HAL_OK) {
      if (HAL_DMA2D_Start(&hDma2dHandler, ColorIndex, (uint32_t)pDst, xSize, ySize) == HAL_OK) {
        /* Polling For DMA transfer */
        while (hDma2dHandler.Instance->CR & DMA2D_CR_START);

        HAL_DMA2D_PollForTransfer(&hDma2dHandler, 10);
      }
    }
  }
}
