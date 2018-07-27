/*******************************************************************************
    Description: This file provides code for the configuration of the CRC instances.
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __crc_H
#define __crc_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "main.h"

extern CRC_HandleTypeDef hcrc;

extern void Error_Handler(void);

void MX_CRC_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__ crc_H */
