/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Src/stm32f7xx_hal_msp.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   HAL MSP module.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "serial.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Private_Functions
    @{
*/

/**
    @brief  Initializes the Global MSP.
    @param  None
    @retval None
*/
void HAL_MspInit(void) {
    /*  NOTE : This function is generated automatically by MicroXplorer and eventually
              modified by the user
    */
}

/**
    @brief  DeInitializes the Global MSP.
    @param  None
    @retval None
*/
void HAL_MspDeInit(void) {
    /*  NOTE : This function is generated automatically by MicroXplorer and eventually
              modified by the user
    */
}

#if (USE_DMA1_SPI == 1)
/**
    @brief SPI MSP Initialization:
           This function configures the SPI hardware resources:
              - Peripherals' clock enable
              - Peripherals' GPIO Configuration
              - DMA configuration for transmission request by peripheral
              - NVIC configuration for DMA interrupt request enable
    @param hspi: SPI handle pointer
    @retval None
*/
void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi) {
    static DMA_HandleTypeDef hdma_tx;
    static DMA_HandleTypeDef hdma_rx;
    GPIO_InitTypeDef  GPIO_InitStruct;
    /* Enable peripherals and GPIO Clocks */
    /* Enable GPIO TX/RX clock */
    SPIx_SCK_GPIO_CLK_ENABLE();
    SPIx_MISO_GPIO_CLK_ENABLE();
    SPIx_MOSI_GPIO_CLK_ENABLE();
    __DSB();
    /* Enable SPI3 clock */
    SPIx_CLK_ENABLE();
    __DSB();
    /* Enable DMA1 clock */
    DMAx_CLK_ENABLE();
    __DSB();
    /* Configure peripheral GPIO */
    /* SPI SCK GPIO pin configuration */
    GPIO_InitStruct.Pin       = SPIx_SCK_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FAST;
    GPIO_InitStruct.Alternate = SPIx_SCK_AF;
    HAL_GPIO_Init(SPIx_SCK_GPIO_PORT, &GPIO_InitStruct);
    /* SPI MISO GPIO pin configuration */
    GPIO_InitStruct.Pin = SPIx_MISO_PIN;
    GPIO_InitStruct.Alternate = SPIx_MISO_AF;
    HAL_GPIO_Init(SPIx_MISO_GPIO_PORT, &GPIO_InitStruct);
    /* SPI MOSI GPIO pin configuration  */
    GPIO_InitStruct.Pin = SPIx_MOSI_PIN;
    GPIO_InitStruct.Alternate = SPIx_MOSI_AF;
    HAL_GPIO_Init(SPIx_MOSI_GPIO_PORT, &GPIO_InitStruct);
    /* Configure the DMA1 streams */
    /* Configure the DMA1 handler for Transmission process */
    hdma_tx.Instance                 = SPIx_TX_DMA_STREAM;
    hdma_tx.Init.Channel             = SPIx_TX_DMA_CHANNEL;
    hdma_tx.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx.Init.Mode                = DMA_NORMAL;
    hdma_tx.Init.Priority            = DMA_PRIORITY_LOW;
    hdma_tx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx.Init.PeriphBurst         = DMA_PBURST_INC4;
    HAL_DMA_Init(&hdma_tx);
    /* Associate the initialized DMA1 handle to the the SPI handle */
    __HAL_LINKDMA(hspi, hdmatx, hdma_tx);
    /* Configure the DMA handler for reception process */
    hdma_rx.Instance                 = SPIx_RX_DMA_STREAM;
    hdma_rx.Init.Channel             = SPIx_RX_DMA_CHANNEL;
    hdma_rx.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    hdma_rx.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_rx.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_rx.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_rx.Init.Mode                = DMA_NORMAL;
    hdma_rx.Init.Priority            = DMA_PRIORITY_HIGH;
    hdma_rx.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_rx.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_rx.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_rx.Init.PeriphBurst         = DMA_PBURST_INC4;
    HAL_DMA_Init(&hdma_rx);
    /* Associate the initialized DMA handle to the the SPI handle */
    __HAL_LINKDMA(hspi, hdmarx, hdma_rx);
#ifdef CHECK_TRANSFER_DMA1_SPI
    /* Configure the NVIC for DMA1 */
    /* NVIC configuration for DMA transfer complete interrupt (SPI3_TX) */
    HAL_NVIC_SetPriority(SPIx_DMA_TX_IRQn, 0, 1);
    HAL_NVIC_EnableIRQ(SPIx_DMA_TX_IRQn);
    /* NVIC configuration for DMA1 transfer complete interrupt (SPI3_RX) */
    HAL_NVIC_SetPriority(SPIx_DMA_RX_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(SPIx_DMA_RX_IRQn);
#endif
}

/**
    @brief SPI MSP De-Initialization
           This function frees the hardware resources used in this example:
             - Disable the Peripheral's clock
             - Revert GPIO, DMA and NVIC configuration to their default state
    @param hspi: SPI handle pointer
    @retval None
*/
void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi) {
    /* Reset SPI peripheral */
    SPIx_FORCE_RESET();
    SPIx_RELEASE_RESET();
    /* De-initialize GPIO pin clocks */
    HAL_GPIO_DeInit(SPIx_SCK_GPIO_PORT, SPIx_SCK_PIN);
    HAL_GPIO_DeInit(SPIx_MISO_GPIO_PORT, SPIx_MISO_PIN);
    HAL_GPIO_DeInit(SPIx_MOSI_GPIO_PORT, SPIx_MOSI_PIN);
    /* Disable the DMA1 Streams */
    /* De-Initialize the DMA Stream associate to transmission process */
    HAL_DMA_DeInit(hspi->hdmatx);
    /* De-Initialize the DMA Stream associate to reception process */
    HAL_DMA_DeInit(hspi->hdmatx);
#ifdef CHECK_TRANSFER_DMA1_SPI
    /* Disable the NVIC for DMA1 */
    HAL_NVIC_DisableIRQ(SPIx_DMA_TX_IRQn);
    HAL_NVIC_DisableIRQ(SPIx_DMA_RX_IRQn);
#endif
}
#endif

#if (CHECK_TRANSFER == 1)
/**
    @brief RNG MSP Initialization
           This function initialize the RNG peripheral:
              - RNG clock enable
    @param hrng: RNG handle pointer
    @retval None
*/
void HAL_RNG_MspInit(RNG_HandleTypeDef *hrng) {
    /* RNG Peripheral clock enable */
    __HAL_RCC_RNG_CLK_ENABLE();
    __DSB();
}

/**
    @brief RNG MSP De-Initialization
    @param hrng: RNG handle pointer
    @retval None
*/
void HAL_RNG_MspDeInit(RNG_HandleTypeDef *hrng) {
    /* Enable RNG reset state */
    __HAL_RCC_RNG_FORCE_RESET();
    /* Release RNG from reset state */
    __HAL_RCC_RNG_RELEASE_RESET();
}
#endif
