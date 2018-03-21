#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "main.h"
#include "color_waves.h"
#include "osc.h"
#include "lcd_log.h"

#define WINDOWX 480
#define WINDOWY 272

// INCREMENT = SCALE / sqrt(N) * 2
// Optimizer-friendly values
#define DIMENSION 32
#define N (DIMENSION*DIMENSION) // Number of dots
#define SCALE 4096
#define INCREMENT (SCALE / DIMENSION * 2)

#define ANGLE_AVERGE 16
#define POWER_HISTORY_DEPTH 32

#define RED_COLORS 255
#define GREEN_COLORS 255
#define BLUE_COLORS 255

#define REDSHIFT 0
#define GREENSHIFT 8
#define BLUESHIFT 16

#define SPEED 10

#define PI2 6.283185307179586476925286766559f

#define LAYER1_BUFFER0  0xC0200000
#define LAYER1_BUFFER1  0xC0400000

#define ACTIVE_LAYER 1

uint32_t non_visible_buffer = LAYER1_BUFFER1;
uint32_t do_switch_buffer = 0;

//SWIM_WINDOW_T win1;

int16_t sine[SCALE];
int16_t cosi[SCALE];

int16_t angleX = 0;
int16_t angleY = 1200;
int16_t angleZ = 0;

int16_t speedX = 10;
int16_t speedY = 20;
int16_t speedZ = 30;

int16_t xyz[3][N];
int16_t dotsMemory[N];

extern int16_t right[];
extern LTDC_HandleTypeDef   hLtdcHandler;

int32_t angleHistory[ANGLE_AVERGE];
int32_t angleAverage;
uint32_t angleHistoryIndex = 0;

uint16_t powerHistory[POWER_HISTORY_DEPTH];
uint32_t powerHistoryIndex = 0;

uint16_t fastsqrt(uint32_t n);
void draw(int16_t xyz[3][N], uint8_t rgb[3][N]) ;
void rotate(int16_t xyz[3][N], uint8_t rgb[3][N],
            uint16_t angleX, uint16_t angleY, uint16_t angleZ);
void matrix(int16_t xyz[3][N], uint8_t rgb[3][N]);

void SetBufferForLDTClayer(uint32_t FBStartAddress, uint16_t LayerIdx);
void BSP_LCD_DrawPixelNonVisibleBuffer(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code);
void BSP_LCD_ClearNonVisibleBuffer(uint32_t Color);
void SwitchDoubleBuffer(void);

/**
  * @brief  switch buffer during vertical blank period
  * @param  none
  * @retval None
  *
  * this function should be used inside LTDC Vblanck interrupt,
  * to switch buffer only during vertical blank period
*/
void InterruptCheckAndSwitchDoubleBuffer(void) {
  if (do_switch_buffer != 0) {
    do_switch_buffer = 0;
    SwitchDoubleBuffer();
  }
}

/**
  * @brief  switch buffers between visible and nonvisible
  * @param  none
  * @retval None
  *
  * switch buffers between visible and nonvisible
*/
void SwitchDoubleBuffer(void) {
  if (non_visible_buffer == LAYER1_BUFFER0) {
    non_visible_buffer = LAYER1_BUFFER1;
    SetBufferForLDTClayer(LAYER1_BUFFER0, ACTIVE_LAYER);
  } else {
    non_visible_buffer = LAYER1_BUFFER0;
    SetBufferForLDTClayer(LAYER1_BUFFER1, ACTIVE_LAYER);
  }
}

/**
  * @brief  set new address for visible area of LTDC layer (frame buffer)
  * @param  FBStartAddress new buffer address
  * @param  LayerIdx LTDC layer number
  * @retval None
  *
  * switch buffers between visible and nonvisible
*/
void SetBufferForLDTClayer(uint32_t FBStartAddress, uint16_t LayerIdx) {
  /* Configures the color frame buffer start address */
  LTDC_LAYER(&hLtdcHandler, LayerIdx)->CFBAR &= ~(LTDC_LxCFBAR_CFBADD);
  LTDC_LAYER(&hLtdcHandler, LayerIdx)->CFBAR = (FBStartAddress);
}

/**
  * @brief  Draws a pixel in the non visible buffer
  * @param  Xpos: X position
  * @param  Ypos: Y position
  * @param  RGB_Code: Pixel color in ARGB mode (8-8-8-8)
  * @retval None
  */
void BSP_LCD_DrawPixelNonVisibleBuffer(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code) {
  /* Write data value to all SDRAM memory */
  if (hLtdcHandler.LayerCfg[ACTIVE_LAYER].PixelFormat == LTDC_PIXEL_FORMAT_RGB565) {
    *(__IO uint16_t *)(non_visible_buffer + (2 * (Ypos * BSP_LCD_GetXSize() + Xpos))) = (uint16_t)RGB_Code;
  } else if (hLtdcHandler.LayerCfg[ACTIVE_LAYER].PixelFormat == LTDC_PIXEL_FORMAT_ARGB4444) {
    *(__IO uint16_t *)(non_visible_buffer + (2 * (Ypos * BSP_LCD_GetXSize() + Xpos))) = (uint16_t)RGB_Code;
  } else {
    *(__IO uint32_t *)(non_visible_buffer + (4 * (Ypos * BSP_LCD_GetXSize() + Xpos))) = RGB_Code;
  }
}

/**
  * @brief  Fill non-visible layer buffer with defined color
  * @param  color: color
  * @retval None
  */
void BSP_LCD_ClearNonVisibleBuffer(uint32_t Color) {
  DMA2D_HandleTypeDef hDma2dHandler;
  /* Clear the LCD */
  //LL_FillBuffer(ACTIVE_LAYER, (uint32_t *)(non_visible_buffer), BSP_LCD_GetXSize(), BSP_LCD_GetYSize(), 0, Color);
  /* Register to memory mode with ARGB8888 as color Mode */
  hDma2dHandler.Init.Mode         = DMA2D_R2M;

  if (hLtdcHandler.LayerCfg[ACTIVE_LAYER].PixelFormat == LTDC_PIXEL_FORMAT_RGB565) {
    hDma2dHandler.Init.ColorMode    = DMA2D_RGB565;
  } else if (hLtdcHandler.LayerCfg[ACTIVE_LAYER].PixelFormat == LTDC_PIXEL_FORMAT_ARGB4444) {
    hDma2dHandler.Init.ColorMode    = DMA2D_ARGB4444;
  } else {
    hDma2dHandler.Init.ColorMode    = DMA2D_ARGB8888;
  }

  hDma2dHandler.Init.OutputOffset = 0;
  hDma2dHandler.Instance = DMA2D;

  /* DMA2D Initialization */
  if (HAL_DMA2D_Init(&hDma2dHandler) == HAL_OK) {
    if (HAL_DMA2D_ConfigLayer(&hDma2dHandler, ACTIVE_LAYER) == HAL_OK) {
      if (HAL_DMA2D_Start(&hDma2dHandler, Color, (uint32_t)non_visible_buffer, BSP_LCD_GetXSize(), BSP_LCD_GetYSize()) == HAL_OK) {
        /* Polling For DMA transfer */
        while (hDma2dHandler.Instance->CR & DMA2D_CR_START);

        HAL_DMA2D_PollForTransfer(&hDma2dHandler, 10);
      }
    }
  }
}

void color_wave_initialize(void) {
  LCD_LayerCfgTypeDef  layer_cfg;
  uint16_t i;

  for (i = 0; i < SCALE; i++) {
    sine[i] = (int)(sin(PI2 * i / SCALE) * SCALE);
    cosi[i] = (int)(cos(PI2 * i / SCALE) * SCALE); // FIXME cos(x) = sin(pi / 2 + x)
  }

  BSP_LCD_LayerDefaultInit(1, LAYER1_BUFFER0);
  /* Layer Init */
  layer_cfg.WindowX0 = 0;
  layer_cfg.WindowX1 = BSP_LCD_GetXSize();
  layer_cfg.WindowY0 = 0;
  layer_cfg.WindowY1 = BSP_LCD_GetYSize();
  layer_cfg.PixelFormat = LTDC_PIXEL_FORMAT_ARGB4444;
  layer_cfg.FBStartAdress = LAYER1_BUFFER0;
  layer_cfg.Alpha = 255;
  layer_cfg.Alpha0 = 0;
  layer_cfg.Backcolor.Blue = 0;
  layer_cfg.Backcolor.Green = 0;
  layer_cfg.Backcolor.Red = 0;
  layer_cfg.BlendingFactor1 = LTDC_BLENDING_FACTOR1_PAxCA;
  layer_cfg.BlendingFactor2 = LTDC_BLENDING_FACTOR2_PAxCA;
  layer_cfg.ImageWidth = BSP_LCD_GetXSize();
  layer_cfg.ImageHeight = BSP_LCD_GetYSize();
  HAL_LTDC_ConfigLayer(&hLtdcHandler, &layer_cfg, ACTIVE_LAYER);
  BSP_LCD_SelectLayer(ACTIVE_LAYER);
}

uint16_t fastsqrt(uint32_t n) {
  uint16_t c = 0x8000;
  uint16_t g = 0x8000;

  for (;;) {
    if (g * g > n) {
      g ^= c;
    }

    c >>= 1;

    if (c == 0) {
      return g;
    }

    g |= c;
  }
}

void matrix(int16_t xyz[3][N], uint8_t rgb[3][N]) {
  static uint32_t t = 0;
  uint16_t i;
  int16_t x = -SCALE;
  int16_t y = -SCALE;
  uint16_t d;
  uint16_t s;
  uint16_t waveHeight;
#if 0
  dotsMemory[0] = 0;

// move one line in history
  for (i = 0; i < N - DIMENSION; i++) {
    dotsMemory[N - i ] = dotsMemory[N - i - DIMENSION];
  }

  for (i = 0; i < DIMENSION; i++) {
    dotsMemory[0] += abs(right[i * NUM_SAMPLES / DIMENSION / 4] / 8);
  }

  for (i = 0; i < DIMENSION; i++) {
    dotsMemory[i] = dotsMemory[0];
  }

#endif

  for (i = 0; i < N; i++) {
    xyz[0][i] = x;
    xyz[1][i] = y;
    //xyz[2][i] = dotsMemory[i] ;
    d = fastsqrt(x * x + y * y);
    s = sine[(t * 30) % SCALE] + SCALE;
//  xyz[2][i] = sine[(d + s) % SCALE] *
//              sine[(t * 10) % SCALE] / SCALE / 2;  //2
    // implement the height of wave depending on audio power and history.
    {
      float distanceCenter;
      float power;
      int32_t index;
      int32_t tempPowerHistoryIndex;
      distanceCenter = (float)d / (float)INCREMENT;
      index = (uint32_t) distanceCenter;
      tempPowerHistoryIndex = powerHistoryIndex - index;

      if (tempPowerHistoryIndex <= 0) {
        tempPowerHistoryIndex = POWER_HISTORY_DEPTH - 1 + tempPowerHistoryIndex;
      }

      power = (float)powerHistory[tempPowerHistoryIndex] *
              (distanceCenter - (float)index);
      tempPowerHistoryIndex--;

      if (tempPowerHistoryIndex <= 0) {
        tempPowerHistoryIndex = POWER_HISTORY_DEPTH - 1;
      }

      power += (float)powerHistory[tempPowerHistoryIndex] *
               (1.0 - (distanceCenter - (float)index));
      xyz[2][i] = (int16_t)power;
    }
    rgb[0][i] = (cosi[xyz[2][i] + SCALE / 2] + SCALE) *
                (RED_COLORS - 1) / SCALE / 2;
    rgb[1][i] = (cosi[(xyz[2][i] + SCALE / 2 + 2 * SCALE / 3) % SCALE] + SCALE) *
                (GREEN_COLORS - 1) / SCALE / 2;
    rgb[2][i] = (cosi[(xyz[2][i] + SCALE / 2 + SCALE / 3) % SCALE] + SCALE) *
                (BLUE_COLORS - 1) / SCALE / 2;
    x += INCREMENT;

    if (x >= SCALE) {
      x = -SCALE;
      y += INCREMENT;
    }
  }

  t++;
}

void rotate(int16_t xyz[3][N], uint8_t rgb[3][N],
            uint16_t angleX, uint16_t angleY, uint16_t angleZ) {
  uint16_t i;
  int16_t tmpX;
  int16_t tmpY;
  int16_t sinx = sine[angleX];
  int16_t cosx = cosi[angleX];
  int16_t siny = sine[angleY];
  int16_t cosy = cosi[angleY];
  int16_t sinz = sine[angleZ];
  int16_t cosz = cosi[angleZ];

  for (i = 0; i < N; i++) {
    tmpX      = (xyz[0][i] * cosx - xyz[2][i] * sinx) / SCALE;
    xyz[2][i] = (xyz[0][i] * sinx + xyz[2][i] * cosx) / SCALE;
    xyz[0][i] = tmpX;
    tmpY      = (xyz[1][i] * cosy - xyz[2][i] * siny) / SCALE;
    xyz[2][i] = (xyz[1][i] * siny + xyz[2][i] * cosy) / SCALE;
    xyz[1][i] = tmpY;
    tmpX      = (xyz[0][i] * cosz - xyz[1][i] * sinz) / SCALE;
    xyz[1][i] = (xyz[0][i] * sinz + xyz[1][i] * cosz) / SCALE;
    xyz[0][i] = tmpX;
  }
}

void draw(int16_t xyz[3][N], uint8_t rgb[3][N]) {
  uint16_t i;
  uint16_t projX;
  uint16_t projY;
  uint16_t projZ;
  uint16_t dotSize;
  BSP_LCD_ClearNonVisibleBuffer(0x0);

  for (i = 0; i < N; i++) {
    projZ   = SCALE - (xyz[2][i] + SCALE) / 4;
    projX   = WINDOWX / 2 + (xyz[0][i] * projZ / SCALE) / 25;
    projY   = WINDOWY / 2 + (xyz[1][i] * projZ / SCALE) / 25;
    dotSize = 3 - (xyz[2][i] + SCALE) * 2 / SCALE;

    if (projX > dotSize &&
        projY > dotSize &&
        projX < WINDOWX - dotSize &&
        projY < WINDOWY - dotSize) {
      if (dotSize > 0) {
        //BSP_LCD_FillCircle(projX, projY, dotSize);
        uint32_t color = (rgb[0][i] << REDSHIFT) +
                         (rgb[1][i] << GREENSHIFT) +
                         (rgb[2][i] << BLUESHIFT) +
                         /* alpha */ (0xFF << 24);
        color = ((rgb[0][i] & 0xF0)  << 4) +
                ((rgb[1][i] & 0xF0) << 0) +
                ((rgb[2][i] & 0xF0) >> 4) +
                /* alpha */ (0xF << 12);
        BSP_LCD_DrawPixelNonVisibleBuffer(projX, projY, color);
        BSP_LCD_DrawPixelNonVisibleBuffer(projX + 1, projY, color);
        BSP_LCD_DrawPixelNonVisibleBuffer(projX, projY + 1, color);
        BSP_LCD_DrawPixelNonVisibleBuffer(projX + 1, projY + 1, color);
      }
    }
  }
}

void WaveDisplay(int16_t angle, int16_t power) {
  int32_t dev_lcd = 0;
  int32_t windowX;
  int32_t windowY;
  uint32_t i;
  uint8_t joyState;
  uint16_t ledState;
  uint8_t rgb[3][N];
  windowX = WINDOWX;
  windowY = WINDOWY;
  angleHistory[angleHistoryIndex] = angle;
  angleHistoryIndex = (angleHistoryIndex + 1) % ANGLE_AVERGE;
  angleAverage = 0;

  for (i = 0; i < ANGLE_AVERGE; i++) {
    angleAverage += angleHistory[i];
  }

  powerHistory[powerHistoryIndex] = power;
  powerHistoryIndex++;

  if (powerHistoryIndex >= POWER_HISTORY_DEPTH) {
    powerHistoryIndex = 0;
  }

  matrix(xyz, rgb);
  //angleZ = (SCALE / 2) + (angleAverage / ANGLE_AVERGE);
  angleZ = ((SCALE * 3 / 4) + (angleAverage / ANGLE_AVERGE)) % SCALE;
  rotate(xyz, rgb, angleX, angleY, angleZ);
  draw(xyz, rgb);
  do_switch_buffer = 1;
}
