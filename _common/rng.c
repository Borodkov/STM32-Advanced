/*******************************************************************************
    Description:
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "rng.h"

RNG_HandleTypeDef hrng;

/* RNG init function */
void MX_RNG_Init(void) {
    hrng.Instance = RNG;

    if (HAL_RNG_Init(&hrng) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_RNG_MspInit(RNG_HandleTypeDef *rngHandle) {
    if (rngHandle->Instance == RNG) {
        __HAL_RCC_RNG_CLK_ENABLE();
    }
}

void HAL_RNG_MspDeInit(RNG_HandleTypeDef *rngHandle) {
    if (rngHandle->Instance == RNG) {
        __HAL_RCC_RNG_CLK_DISABLE();
    }
}
