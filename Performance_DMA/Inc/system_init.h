/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Inc/system_init.h
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   Header for system_init.c module
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSTEM_INIT_H
#define __SYSTEM_INIT_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "main.h"  /* USE_DMA2, USE_DMA1_SPI */

/* Exported types ------------------------------------------------------------*/
#if (USE_DMA2 == 1)
/* DMA handler declaration */
extern DMA_HandleTypeDef   DmaHandle;
#endif

#if (CHECK_TRANSFER == 1)
/* Set to 1 if transfer is correctly completed */
extern __IO uint8_t FlagDMA2TransferComplete;
extern __IO uint8_t FlagDMA1SPITransferComplete;
#endif

#if (USE_DMA1_SPI == 1)
extern SPI_HandleTypeDef SpiHandle;
#endif

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void sys_init(void);

#endif /* __SYSTEM_INIT__H */
