/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Inc/stm32f7xx_it.h
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This file contains the headers of the interrupt handlers.
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __STM32F7xx_IT_H
#define __STM32F7xx_IT_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Exported types ------------------------------------------------------------*/
extern __IO uint32_t BenchmarkTick;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

void NMI_Handler(void);
void HardFault_Handler(void);
void MemManage_Handler(void);
void BusFault_Handler(void);
void UsageFault_Handler(void);
void SVC_Handler(void);
void DebugMon_Handler(void);
void PendSV_Handler(void);
void SysTick_Handler(void);
#ifdef CHECK_TRANSFER_DMA2
void DMA2_INSTANCE_IRQHANDLER(void);
#endif /* CHECK_TRANSFER_DMA2 */
#ifdef CHECK_TRANSFER_DMA1_SPI
void SPIx_DMA_RX_IRQHandler(void);
void SPIx_DMA_TX_IRQHandler(void);
#endif /* CHECK_TRANSFER_DMA1_SPI */

#ifdef __cplusplus
}
#endif

#endif /* __STM32F7xx_IT_H */
