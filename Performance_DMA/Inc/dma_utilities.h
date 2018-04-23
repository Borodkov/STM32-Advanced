/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Inc/dma_utilities.h
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   Header that contains all the definitions to use DMAs
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __DMA_UTILITIES_H
#define __DMA_UTILITIES_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "main.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* DO NOT modify these definitions */
#define FLASHAXI_DMA_START_ADDRESS       0x080C0000U
#define FLASHTCM_DMA_START_ADDRESS       0x002C0000U
#define DTCMRAM_DMA_START_ADDRESS        0x20002400U
#define SRAM1_DMA_START_ADDRESS          0x20012400U
#define SRAM2_DMA_START_ADDRESS          0x2004C000U
#define SDRAM_DMA_START_ADDRESS          0x60000000U

#define DTCMRAM_END_ADDRESS              0x2000FFFFU
#define SRAM1_END_ADDRESS                0x2004BFFFU
#define SRAM2_END_ADDRESS                0x2004FFFFU

#define DTCMRAM_DMA_AVAILABLE_SIZE       (DTCMRAM_END_ADDRESS - DTCMRAM_DMA_START_ADDRESS + 1)
#define SRAM1_DMA_AVAILABLE_SIZE         (SRAM1_END_ADDRESS - SRAM1_DMA_START_ADDRESS + 1)
#define SRAM2_DMA_AVAILABLE_SIZE         (SRAM2_END_ADDRESS - SRAM2_DMA_START_ADDRESS + 1)
#define SDRAM_DMA_AVAILABLE_SIZE         (0xFFFFU) /* 64KB */
#define FLASHAXI_DMA_AVAILABLE_SIZE      (0xFFFFU) /* 64KB */
#define FLASHTCM_DMA_AVAILABLE_SIZE      (0xFFFFU) /* 64KB */

#define FLAG_END_DMA2_TRANSFER           0x1
#define FLAG_END_DMA1_TRANSFER           0x2
#define FLAG_END_FFT_PROCESS             0x4

/* Exported macro ------------------------------------------------------------*/
#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))
/* Exported functions ------------------------------------------------------- */

#endif /*__DMA_UTILITIES_H */
