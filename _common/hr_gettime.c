/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "hr_gettime.h"

#include "stm32f7xx_hal.h"

TIM_HandleTypeDef tim2_handle;

static const uint32_t ulReloadCount = 10000000ul;
static const uint32_t ulPrescale = 100ul;

static uint32_t ulInterruptCount = 0;

uint32_t ulTimer2Flags;
void TIM2_IRQHandler(void) {
    ulTimer2Flags = tim2_handle.Instance->SR;

    if ((ulTimer2Flags & TIM_FLAG_UPDATE) != 0) {
        __HAL_TIM_CLEAR_FLAG(&tim2_handle, TIM_FLAG_UPDATE);
        ulInterruptCount++;
    }
}

/* Timer2 initialization function */
void vStartHighResolutionTimer(void) {
    /* TIM2 clock enable */
    __HAL_RCC_TIM2_CLK_ENABLE();
    tim2_handle.Instance = TIM2; /* Register base address             */
    tim2_handle.Init.Prescaler = (ulPrescale - 1ul);         /* Specifies the prescaler value used to divide the TIM clock. */
    tim2_handle.Init.CounterMode = TIM_COUNTERMODE_UP;       /* Specifies the counter mode. */
    tim2_handle.Init.Period = (ulReloadCount - 1ul);         /* Specifies the period value to be loaded into the active. */
    tim2_handle.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1; /* Specifies the clock division. */
    tim2_handle.Init.RepetitionCounter = 0ul;                /* Specifies the repetition counter value. */
    tim2_handle.Channel = HAL_TIM_ACTIVE_CHANNEL_1;
    HAL_TIM_Base_Init(&tim2_handle);
    HAL_NVIC_SetPriority(TIM2_IRQn, configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY + 1, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    // clear flag before start TIM
    __HAL_TIM_CLEAR_FLAG(&tim2_handle, TIM_FLAG_UPDATE);
    HAL_TIM_Base_Start_IT(&tim2_handle);
    ulTimer2Flags = tim2_handle.Instance->SR;
}

uint64_t ullGetHighResolutionTime() {
    uint64_t ullReturn;

    if (tim2_handle.Instance == NULL) {
        ullReturn = 1000ull * xTaskGetTickCount();
    } else {
        uint32_t ulCounts[2];
        uint32_t ulSlowCount;

        for (;;) {
            ulCounts[0] = tim2_handle.Instance->CNT;
            ulSlowCount = ulInterruptCount;
            ulCounts[1] = tim2_handle.Instance->CNT;

            if (ulCounts[1] >= ulCounts[0]) {
                /* TIM2_IRQHandler() has not occurred in between. */
                break;
            }
        }

        ullReturn = (uint64_t)ulSlowCount * ulReloadCount + ulCounts[1];
    }

    return ullReturn;
}
