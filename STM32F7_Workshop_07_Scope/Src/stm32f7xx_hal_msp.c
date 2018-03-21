/**
  ******************************************************************************
    File Name          : stm32f7xx_hal_msp.c
    Description        : This file provides code for the MSP Initialization
                         and de-Initialization codes.
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

extern DMA_HandleTypeDef hdma_adc3;

/**
    Initializes the Global MSP.
*/
void HAL_MspInit(void) {
    HAL_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);
    /* System interrupt init*/
    /* SysTick_IRQn interrupt configuration */
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void HAL_ADC_MspInit(ADC_HandleTypeDef *hadc) {
    GPIO_InitTypeDef GPIO_InitStruct;

    if (hadc->Instance == ADC3) {
        /* Peripheral clock enable */
        __ADC3_CLK_ENABLE();
        /** ADC3 GPIO Configuration
            PA0/WKUP     ------> ADC3_IN0
        */
        GPIO_InitStruct.Pin = GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        /* Peripheral DMA init*/
        hdma_adc3.Instance = DMA2_Stream1;
        hdma_adc3.Init.Channel = DMA_CHANNEL_2;
        hdma_adc3.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_adc3.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_adc3.Init.MemInc = DMA_MINC_ENABLE;
        hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_adc3.Init.Mode = DMA_CIRCULAR;
        hdma_adc3.Init.Priority = DMA_PRIORITY_LOW;
        hdma_adc3.Init.FIFOMode = DMA_FIFOMODE_DISABLE;
        HAL_DMA_Init(&hdma_adc3);
        __HAL_LINKDMA(hadc, DMA_Handle, hdma_adc3);
        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(ADC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(ADC_IRQn);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef *hadc) {
    if (hadc->Instance == ADC3) {
        /* Peripheral clock disable */
        __ADC3_CLK_DISABLE();
        /** ADC3 GPIO Configuration
            PA0/WKUP     ------> ADC3_IN0
        */
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0);
        /* Peripheral DMA DeInit*/
        HAL_DMA_DeInit(hadc->DMA_Handle);
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(ADC_IRQn);
    }
}

void HAL_DMA2D_MspInit(DMA2D_HandleTypeDef *hdma2d) {
    if (hdma2d->Instance == DMA2D) {
        /* Peripheral clock enable */
        __DMA2D_CLK_ENABLE();
    }
}

void HAL_DMA2D_MspDeInit(DMA2D_HandleTypeDef *hdma2d) {
    if (hdma2d->Instance == DMA2D) {
        /* Peripheral clock disable */
        __DMA2D_CLK_DISABLE();
    }
}

void HAL_LTDC_MspInit(LTDC_HandleTypeDef *hltdc) {
    GPIO_InitTypeDef GPIO_InitStruct;

    if (hltdc->Instance == LTDC) {
        /* Peripheral clock enable */
        __LTDC_CLK_ENABLE();
        /** LTDC GPIO Configuration
            PE4     ------> LTDC_B0
            PJ13     ------> LTDC_B1
            PK7     ------> LTDC_DE
            PK6     ------> LTDC_B7
            PK5     ------> LTDC_B6
            PG12     ------> LTDC_B4
            PJ14     ------> LTDC_B2
            PI10     ------> LTDC_HSYNC
            PK4     ------> LTDC_B5
            PJ15     ------> LTDC_B3
            PI9     ------> LTDC_VSYNC
            PK1     ------> LTDC_G6
            PK2     ------> LTDC_G7
            PI15     ------> LTDC_R0
            PJ11     ------> LTDC_G4
            PK0     ------> LTDC_G5
            PI14     ------> LTDC_CLK
            PJ8     ------> LTDC_G1
            PJ10     ------> LTDC_G3
            PJ7     ------> LTDC_G0
            PJ9     ------> LTDC_G2
            PJ6     ------> LTDC_R7
            PJ4     ------> LTDC_R5
            PJ5     ------> LTDC_R6
            PJ3     ------> LTDC_R4
            PJ2     ------> LTDC_R3
            PJ0     ------> LTDC_R1
            PJ1     ------> LTDC_R2
        */
        GPIO_InitStruct.Pin = GPIO_PIN_4;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_11
                              | GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_7 | GPIO_PIN_9
                              | GPIO_PIN_6 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_3
                              | GPIO_PIN_2 | GPIO_PIN_0 | GPIO_PIN_1;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOJ, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4
                              | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_0;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_12;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF9_LTDC;
        HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_15 | GPIO_PIN_14;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
        GPIO_InitStruct.Alternate = GPIO_AF14_LTDC;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
        /* Peripheral interrupt init*/
        HAL_NVIC_SetPriority(LTDC_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(LTDC_IRQn);
    }
}

void HAL_LTDC_MspDeInit(LTDC_HandleTypeDef *hltdc) {
    if (hltdc->Instance == LTDC) {
        /* Peripheral clock disable */
        __LTDC_CLK_DISABLE();
        /** LTDC GPIO Configuration
            PE4     ------> LTDC_B0
            PJ13     ------> LTDC_B1
            PK7     ------> LTDC_DE
            PK6     ------> LTDC_B7
            PK5     ------> LTDC_B6
            PG12     ------> LTDC_B4
            PJ14     ------> LTDC_B2
            PI10     ------> LTDC_HSYNC
            PK4     ------> LTDC_B5
            PJ15     ------> LTDC_B3
            PI9     ------> LTDC_VSYNC
            PK1     ------> LTDC_G6
            PK2     ------> LTDC_G7
            PI15     ------> LTDC_R0
            PJ11     ------> LTDC_G4
            PK0     ------> LTDC_G5
            PI14     ------> LTDC_CLK
            PJ8     ------> LTDC_G1
            PJ10     ------> LTDC_G3
            PJ7     ------> LTDC_G0
            PJ9     ------> LTDC_G2
            PJ6     ------> LTDC_R7
            PJ4     ------> LTDC_R5
            PJ5     ------> LTDC_R6
            PJ3     ------> LTDC_R4
            PJ2     ------> LTDC_R3
            PJ0     ------> LTDC_R1
            PJ1     ------> LTDC_R2
        */
        HAL_GPIO_DeInit(GPIOE, GPIO_PIN_4);
        HAL_GPIO_DeInit(GPIOJ, GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_11
                        | GPIO_PIN_8 | GPIO_PIN_10 | GPIO_PIN_7 | GPIO_PIN_9
                        | GPIO_PIN_6 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_3
                        | GPIO_PIN_2 | GPIO_PIN_0 | GPIO_PIN_1);
        HAL_GPIO_DeInit(GPIOK, GPIO_PIN_7 | GPIO_PIN_6 | GPIO_PIN_5 | GPIO_PIN_4
                        | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_0);
        HAL_GPIO_DeInit(GPIOG, GPIO_PIN_12);
        HAL_GPIO_DeInit(GPIOI, GPIO_PIN_10 | GPIO_PIN_9 | GPIO_PIN_15 | GPIO_PIN_14);
        /* Peripheral interrupt DeInit*/
        HAL_NVIC_DisableIRQ(LTDC_IRQn);
    }
}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim_base) {
    GPIO_InitTypeDef GPIO_InitStruct;

    if (htim_base->Instance == TIM2) {
        /* Peripheral clock enable */
        __TIM2_CLK_ENABLE();
    } else if (htim_base->Instance == TIM8) {
        /* Peripheral clock enable */
        __TIM8_CLK_ENABLE();
        /** TIM8 GPIO Configuration
            PI2     ------> TIM8_CH4
        */
        GPIO_InitStruct.Pin = GPIO_PIN_2;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
        HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *htim_base) {
    if (htim_base->Instance == TIM2) {
        /* Peripheral clock disable */
        __TIM2_CLK_DISABLE();
    } else if (htim_base->Instance == TIM8) {
        /* Peripheral clock disable */
        __TIM8_CLK_DISABLE();
        /** TIM8 GPIO Configuration
            PI2     ------> TIM8_CH4
        */
        HAL_GPIO_DeInit(GPIOI, GPIO_PIN_2);
    }
}

static int FMC_Initialized = 0;

static void HAL_FMC_MspInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct;

    if (FMC_Initialized) {
        return;
    }

    FMC_Initialized = 1;
    /* Peripheral clock enable */
    __FMC_CLK_ENABLE();
    /** FMC GPIO Configuration
        PE1   ------> FMC_NBL1
        PE0   ------> FMC_NBL0
        PG15   ------> FMC_SDNCAS
        PD0   ------> FMC_D2
        PD1   ------> FMC_D3
        PF0   ------> FMC_A0
        PF1   ------> FMC_A1
        PF2   ------> FMC_A2
        PF3   ------> FMC_A3
        PG8   ------> FMC_SDCLK
        PF4   ------> FMC_A4
        PH5   ------> FMC_SDNWE
        PH3   ------> FMC_SDNE0
        PF5   ------> FMC_A5
        PD15   ------> FMC_D1
        PD10   ------> FMC_D15
        PC3   ------> FMC_SDCKE0
        PD14   ------> FMC_D0
        PD9   ------> FMC_D14
        PD8   ------> FMC_D13
        PF12   ------> FMC_A6
        PG1   ------> FMC_A11
        PF15   ------> FMC_A9
        PF13   ------> FMC_A7
        PG0   ------> FMC_A10
        PE8   ------> FMC_D5
        PG5   ------> FMC_BA1
        PG4   ------> FMC_BA0
        PF14   ------> FMC_A8
        PF11   ------> FMC_SDNRAS
        PE9   ------> FMC_D6
        PE11   ------> FMC_D8
        PE14   ------> FMC_D11
        PE7   ------> FMC_D4
        PE10   ------> FMC_D7
        PE12   ------> FMC_D9
        PE15   ------> FMC_D12
        PE13   ------> FMC_D10
    */
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_8 | GPIO_PIN_9
                          | GPIO_PIN_11 | GPIO_PIN_14 | GPIO_PIN_7 | GPIO_PIN_10
                          | GPIO_PIN_12 | GPIO_PIN_15 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_15 | GPIO_PIN_8 | GPIO_PIN_1 | GPIO_PIN_0
                          | GPIO_PIN_5 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_15 | GPIO_PIN_10
                          | GPIO_PIN_14 | GPIO_PIN_9 | GPIO_PIN_8;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                          | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_15
                          | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_11;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *hsdram) {
    HAL_FMC_MspInit();
}

static int FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void) {
    if (FMC_DeInitialized) {
        return;
    }

    FMC_DeInitialized = 1;
    /* Peripheral clock enable */
    __FMC_CLK_DISABLE();
    /** FMC GPIO Configuration
        PE1   ------> FMC_NBL1
        PE0   ------> FMC_NBL0
        PG15   ------> FMC_SDNCAS
        PD0   ------> FMC_D2
        PD1   ------> FMC_D3
        PF0   ------> FMC_A0
        PF1   ------> FMC_A1
        PF2   ------> FMC_A2
        PF3   ------> FMC_A3
        PG8   ------> FMC_SDCLK
        PF4   ------> FMC_A4
        PH5   ------> FMC_SDNWE
        PH3   ------> FMC_SDNE0
        PF5   ------> FMC_A5
        PD15   ------> FMC_D1
        PD10   ------> FMC_D15
        PC3   ------> FMC_SDCKE0
        PD14   ------> FMC_D0
        PD9   ------> FMC_D14
        PD8   ------> FMC_D13
        PF12   ------> FMC_A6
        PG1   ------> FMC_A11
        PF15   ------> FMC_A9
        PF13   ------> FMC_A7
        PG0   ------> FMC_A10
        PE8   ------> FMC_D5
        PG5   ------> FMC_BA1
        PG4   ------> FMC_BA0
        PF14   ------> FMC_A8
        PF11   ------> FMC_SDNRAS
        PE9   ------> FMC_D6
        PE11   ------> FMC_D8
        PE14   ------> FMC_D11
        PE7   ------> FMC_D4
        PE10   ------> FMC_D7
        PE12   ------> FMC_D9
        PE15   ------> FMC_D12
        PE13   ------> FMC_D10
    */
    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_1 | GPIO_PIN_0 | GPIO_PIN_8 | GPIO_PIN_9
                    | GPIO_PIN_11 | GPIO_PIN_14 | GPIO_PIN_7 | GPIO_PIN_10
                    | GPIO_PIN_12 | GPIO_PIN_15 | GPIO_PIN_13);
    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_15 | GPIO_PIN_8 | GPIO_PIN_1 | GPIO_PIN_0
                    | GPIO_PIN_5 | GPIO_PIN_4);
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_15 | GPIO_PIN_10
                    | GPIO_PIN_14 | GPIO_PIN_9 | GPIO_PIN_8);
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3
                    | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_12 | GPIO_PIN_15
                    | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_11);
    HAL_GPIO_DeInit(GPIOH, GPIO_PIN_5 | GPIO_PIN_3);
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_3);
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *hsdram) {
    HAL_FMC_MspDeInit();
}
