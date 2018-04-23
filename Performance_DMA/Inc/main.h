/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Inc/main.h
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   Header for main.c module
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stdio.h"
#include "print_config.h"
#include "serial.h"
#include "dma_utilities.h"

/* Exported types ------------------------------------------------------------*/
typedef struct {
    uint32_t cycles_rslt;
    uint32_t cycles_systick;
    uint8_t  flags;
} info;

extern info Infos;

/* Exported constants --------------------------------------------------------*/
#define USE_DMA2                     0
#define USE_DMA1_SPI                 0
#define CHECK_TRANSFER               0

#if (CHECK_TRANSFER == 1)
#if (USE_DMA2 == 1)
#define CHECK_TRANSFER_DMA2
#endif

#if (USE_DMA1_SPI == 1)
#define CHECK_TRANSFER_DMA1_SPI
#endif

#if ((USE_DMA2 == 0) && (USE_DMA1_SPI == 0))
#error WARNING: Please specify at least one DMA to be used
#endif

#endif /* (CHECK_TRANSFER == 1) */

/*** DMA2 defines *************************************************************/
#if ( USE_DMA2 == 1)
#define DMA2_SRC_ADDRESS            SRAM1_DMA_START_ADDRESS
#define DMA2_DST_ADDRESS            DTCMRAM_DMA_START_ADDRESS
#define DMA2_TRANSFER_SIZE          MIN(SRAM1_DMA_AVAILABLE_SIZE,(DTCMRAM_DMA_AVAILABLE_SIZE*6/7))

#define DMA2_INSTANCE               DMA2_Stream3
#define DMA2_CHANNEL                DMA_CHANNEL_3
#define DMA2_INSTANCE_IRQ           DMA2_Stream3_IRQn
#define DMA2_INSTANCE_IRQHANDLER    DMA2_Stream3_IRQHandler

#if (DMA2_TRANSFER_SIZE > 0xFFFF)
#undef DMA2_TRANSFER_SIZE
#define DMA2_TRANSFER_SIZE 0xFFFF
#endif
#endif /* ( USE_DMA2 == 1) */

/*** DMA1/SPI defines *********************************************************/
#if ( USE_DMA1_SPI == 1)
#define DMA1SPI_SRC_ADDRESS         SRAM2_DMA_START_ADDRESS
#define DMA1SPI_DST_ADDRESS         (DTCMRAM_DMA_START_ADDRESS+((DTCMRAM_DMA_AVAILABLE_SIZE*6/7)))
#define DMA1SPI_TRANSFER_SIZE       MIN(SRAM2_DMA_AVAILABLE_SIZE,(DTCMRAM_DMA_AVAILABLE_SIZE/7))

#if (DMA1SPI_TRANSFER_SIZE > 0xFFFF)
#undef DMA1SPI_TRANSFER_SIZE
#define DMA1SPI_TRANSFER_SIZE 0xFFFF
#endif

/*  User can use this section to tailor SPIx instance used and associated
    resources */
/* Definition for SPIx clock resources */
#define SPIx                             SPI3
#define SPIx_CLK_ENABLE()                __HAL_RCC_SPI3_CLK_ENABLE()
#define DMAx_CLK_ENABLE()                __HAL_RCC_DMA1_CLK_ENABLE()
#define SPIx_SCK_GPIO_CLK_ENABLE()       __HAL_RCC_GPIOB_CLK_ENABLE()
#define SPIx_MISO_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()
#define SPIx_MOSI_GPIO_CLK_ENABLE()      __HAL_RCC_GPIOC_CLK_ENABLE()

#define SPIx_FORCE_RESET()               __HAL_RCC_SPI3_FORCE_RESET()
#define SPIx_RELEASE_RESET()             __HAL_RCC_SPI3_RELEASE_RESET()

/* Definition for SPIx Pins */
#define SPIx_SCK_PIN                     GPIO_PIN_3
#define SPIx_SCK_GPIO_PORT               GPIOB
#define SPIx_SCK_AF                      GPIO_AF6_SPI3
#define SPIx_MISO_PIN                    GPIO_PIN_11
#define SPIx_MISO_GPIO_PORT              GPIOC
#define SPIx_MISO_AF                     GPIO_AF6_SPI3
#define SPIx_MOSI_PIN                    GPIO_PIN_12
#define SPIx_MOSI_GPIO_PORT              GPIOC
#define SPIx_MOSI_AF                     GPIO_AF6_SPI3

/* Definition for SPIx's DMA */
#define SPIx_TX_DMA_CHANNEL              DMA_CHANNEL_0
#define SPIx_TX_DMA_STREAM               DMA1_Stream5

#define SPIx_RX_DMA_CHANNEL              DMA_CHANNEL_0
#define SPIx_RX_DMA_STREAM               DMA1_Stream0

#ifdef CHECK_TRANSFER_DMA1_SPI
/* Definition for SPIx's NVIC */
#define SPIx_DMA_TX_IRQn                 DMA1_Stream5_IRQn
#define SPIx_DMA_RX_IRQn                 DMA1_Stream0_IRQn
#define SPIx_DMA_TX_IRQHandler           DMA1_Stream5_IRQHandler
#define SPIx_DMA_RX_IRQHandler           DMA1_Stream0_IRQHandler
#endif
#endif  /* ( USE_DMA1_SPI == 1) */

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */
