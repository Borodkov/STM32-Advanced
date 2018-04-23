/**
 ******************************************************************************
    @file    stm32f7_performances_DMAs/Src/end.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This file contains the routine that ends the banchmark and printfs
            the results.
 ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "serial.h"
#include "main.h"
#include "end.h"
#include "system_init.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
#if defined (CHECK_TRANSFER_DMA2) || defined(CHECK_TRANSFER_DMA1_SPI)
static uint32_t BufferCmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint32_t BufferLength);
#endif

/* Private functions ---------------------------------------------------------*/

/**
    @brief  End of the process: check of the data and display the results
    @param  None
    @retval None
*/
void end(void) {
#if (CHECK_TRANSFER == 1)
    static __IO uint8_t dma1_spi_transfer_status = 0; /* 0 = OK, 1 = KO*/
    static __IO uint8_t dma2_transfer_status = 0; /* 0 = OK, 1 = KO*/
#endif
#ifdef CHECK_TRANSFER_DMA1_SPI
    __IO uint32_t cmp_dma1 = 0;
#endif
#ifdef CHECK_TRANSFER_DMA2
    __IO uint32_t cmp_dma2 = 0;
#endif
    /*  Reconfigure systick as time base source and configure 1ms tick since its
        config has chaged previously */
    HAL_InitTick(TICK_INT_PRIORITY);
    /* Init printf target  */
    Printf_Init();
    /* Print the system configuration of the current project configuration */
    Print_config();
#ifdef CHECK_TRANSFER_DMA2

    /* wait until DMA2 transfer complete */
    while (FlagDMA2TransferComplete == 0);

    /* Check the data transfer integrity done by DMA2 */
    cmp_dma2 = BufferCmp((uint8_t *)((uint32_t *)DMA2_SRC_ADDRESS), \
                         (uint8_t *)((uint32_t *)(DMA2_DST_ADDRESS)), DMA2_TRANSFER_SIZE) ;

    if (cmp_dma2 != 0) {
        /* Turn LED_RED on: Transfer KO */
        BSP_LED_On(LED_RED);
        dma2_transfer_status = 1;
    } else {
        /* Turn LED_GREEN on: Transfer correct */
        BSP_LED_On(LED_GREEN);
    }

#endif
#ifdef CHECK_TRANSFER_DMA1_SPI

    /* Wait for the end of the transfer */
    while ((HAL_SPI_GetState(&SpiHandle) != HAL_SPI_STATE_READY) && (FlagDMA1SPITransferComplete == 0)) {
    }

    /* Check the data transfer integrity done by DMA1 */
    cmp_dma1 = BufferCmp((uint8_t *)((uint32_t *)DMA1SPI_SRC_ADDRESS), \
                         (uint8_t *)((uint32_t *)DMA1SPI_DST_ADDRESS), DMA1SPI_TRANSFER_SIZE);

    if (cmp_dma1 != 0) {
        /* Turn LED_RED on: Transfer error in transmission process */
        BSP_LED_On(LED_RED);
        dma1_spi_transfer_status = 1;
    } else {
        /* Turn LED_GREEN on: Transfer correct */
        BSP_LED_On(LED_GREEN);
    }

#endif
#if (CHECK_TRANSFER == 1)

    /* Display Error message when a DMA transfer is KO */
    if (dma1_spi_transfer_status || dma2_transfer_status) {
        printf("Transfer KO: ");
#ifdef CHECK_TRANSFER_DMA1_SPI

        if (dma1_spi_transfer_status) {
            printf(" GP-DMA1: %lu", cmp_dma1);
        }

#endif
#ifdef CHECK_TRANSFER_DMA2

        if (dma2_transfer_status) {
            printf(" GP-DMA2: %lu", cmp_dma2);
        }

#endif
        printf("\n\r");
    }

    /* Display Error message when a DMA transfer completed before FFT process */
    if (Infos.flags != FLAG_END_FFT_PROCESS) {
        printf("DMAs finished before FFT prcess: \n\r");

        if (Infos.flags & FLAG_END_DMA1_TRANSFER) {
            printf(" GP-DMA1");
        }

        if (Infos.flags & FLAG_END_DMA2_TRANSFER) {
            printf(" GP-DMA2");
        }
    }

#endif
#if (CHECK_TRANSFER != 1)
    /* Display the results on the LCD-TFT */
    printf("                            ----------- \n\r");
    printf("- Total FFT cycles number  | %9lu |\n\r", Infos.cycles_rslt);
    printf("                            ----------- \n\r");
#endif

    while (1) {
#if (CHECK_TRANSFER == 1)

        if (Infos.flags != FLAG_END_FFT_PROCESS) {
            BSP_LED_Toggle(LED_RED);
            HAL_Delay(500);
        }

#endif  /* (CHECK_TRANSFER == 1) */
    }
}

#if defined (CHECK_TRANSFER_DMA2) || defined(CHECK_TRANSFER_DMA1_SPI)
/**
    @brief  Compares two buffers.
    @param  pBuffer1, pBuffer2: buffers to be compared.
    @param  BufferLength: buffer's length
    @retval 0: pBuffer identical to pBuffer1
            Different of 0: the offset of the wrong data.
*/
static uint32_t BufferCmp(uint8_t *pBuffer1, uint8_t *pBuffer2, uint32_t BufferLength) {
    __IO uint32_t len = BufferLength;

    while (len--) {
        if (*pBuffer1 != *pBuffer2) {
            return (BufferLength - len);
        }

        pBuffer1++;
        pBuffer2++;
    }

    return 0;
}
#endif
