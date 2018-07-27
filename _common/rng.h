/*******************************************************************************
    Description:
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __rng_H
#define __rng_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "main.h"

extern RNG_HandleTypeDef hrng;

extern void Error_Handler(void);

void MX_RNG_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__ rng_H */
