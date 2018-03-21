/*******************************************************************************
    @file    bsp_driver_sd.c for F7 (based on stm32756g_eval_sd.c)
    @brief   This file includes a generic uSD card driver.
  *****************************************************************************/

#define BUS_4BITS 1

/* Includes ------------------------------------------------------------------*/
#include "bsp_driver_sd.h"

/* Extern variables ---------------------------------------------------------*/
extern SD_HandleTypeDef _HSD;

/**
    @brief  Initializes the SD card device.
    @retval SD status
*/
uint8_t BSP_SD_Init(void) {
    uint8_t sd_state = MSD_OK;

    /* Check if the SD card is plugged in the slot */
    if (BSP_SD_IsDetected() != SD_PRESENT) {
        return MSD_ERROR_SD_NOT_PRESENT;
    }

    /* HAL SD initialization */
    sd_state = HAL_SD_Init(&_HSD);
#ifdef BUS_4BITS

    /* Configure SD Bus width */
    if (sd_state == MSD_OK) {
        /* Enable wide operation */
        if (HAL_SD_ConfigWideBusOperation(&_HSD, SDMMC_BUS_WIDE_4B) != HAL_OK) {
            sd_state = MSD_ERROR;
        }
    }

#endif
    return sd_state;
}

/**
    @brief  Configures Interrupt mode for SD detection pin.
    @retval Returns 0 in success otherwise 1.
*/
uint8_t BSP_SD_ITConfig(void) {
    /* TBI: add user code here depending on the hardware configuration used */
    return (uint8_t)0;
}

/**
    @brief  Reads block(s) from a specified address in an SD card, in polling mode.
    @param  pData: Pointer to the buffer that will contain the data to transmit
    @param  ReadAddr: Address from where data is to be read
    @param  NumOfBlocks: Number of SD blocks to read
    @param  Timeout: Timeout for read operation
    @retval SD status
*/
uint8_t BSP_SD_ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout) {
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_ReadBlocks(&_HSD, (uint8_t *)pData, ReadAddr, NumOfBlocks, Timeout) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
    @brief  Writes block(s) to a specified address in an SD card, in polling mode.
    @param  pData: Pointer to the buffer that will contain the data to transmit
    @param  WriteAddr: Address from where data is to be written
    @param  NumOfBlocks: Number of SD blocks to write
    @param  Timeout: Timeout for write operation
    @retval SD status
*/
uint8_t BSP_SD_WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout) {
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_WriteBlocks(&_HSD, (uint8_t *)pData, WriteAddr, NumOfBlocks, Timeout) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
    @brief  Reads block(s) from a specified address in an SD card, in DMA mode.
    @param  pData: Pointer to the buffer that will contain the data to transmit
    @param  ReadAddr: Address from where data is to be read
    @param  NumOfBlocks: Number of SD blocks to read
    @retval SD status
*/
uint8_t BSP_SD_ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks) {
    uint8_t sd_state = MSD_OK;

    /* Read block(s) in DMA transfer mode */
    if (HAL_SD_ReadBlocks_DMA(&_HSD, (uint8_t *)pData, ReadAddr, NumOfBlocks) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
    @brief  Writes block(s) to a specified address in an SD card, in DMA mode.
    @param  pData: Pointer to the buffer that will contain the data to transmit
    @param  WriteAddr: Address from where data is to be written
    @param  NumOfBlocks: Number of SD blocks to write
    @retval SD status
*/
uint8_t BSP_SD_WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks) {
    uint8_t sd_state = MSD_OK;

    /* Write block(s) in DMA transfer mode */
    if (HAL_SD_WriteBlocks_DMA(&_HSD, (uint8_t *)pData, WriteAddr, NumOfBlocks) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
    @brief  Erases the specified memory area of the given SD card.
    @param  StartAddr: Start byte address
    @param  EndAddr: End byte address
    @retval SD status
*/
uint8_t BSP_SD_Erase(uint32_t StartAddr, uint32_t EndAddr) {
    uint8_t sd_state = MSD_OK;

    if (HAL_SD_Erase(&_HSD, StartAddr, EndAddr) != HAL_OK) {
        sd_state = MSD_ERROR;
    }

    return sd_state;
}

/**
    @brief  Gets the current SD card data status.
    @param  None
    @retval Data transfer state.
             This value can be one of the following values:
               @arg  SD_TRANSFER_OK: No data transfer is acting
               @arg  SD_TRANSFER_BUSY: Data transfer is acting
*/
uint8_t BSP_SD_GetCardState(void) {
    return ((HAL_SD_GetCardState(&_HSD) == HAL_SD_CARD_TRANSFER) ? SD_TRANSFER_OK : SD_TRANSFER_BUSY);
}

/**
    @brief  Get SD information about specific SD card.
    @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
    @retval None
*/
void BSP_SD_GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo) {
    /* Get SD card Information */
    HAL_SD_GetCardInfo(&_HSD, CardInfo);
}

/**
    @brief  Detects if SD card is correctly plugged in the memory slot or not.
    @param  None
    @retval Returns if SD is detected or not
*/
uint8_t BSP_SD_IsDetected(void) {
    __IO uint8_t status = SD_PRESENT;
    /* USER CODE BEGIN 1 */
    /* user code can be inserted here */
    /* USER CODE END 1 */
    return status;
}
