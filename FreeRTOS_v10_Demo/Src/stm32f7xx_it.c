/**
  ******************************************************************************
    @file    stm32f7xx_it.c
    @brief   Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "cmsis_os.h"

/* External variables --------------------------------------------------------*/
extern TIM_HandleTypeDef htim14;
extern osSemaphoreId xBinSem_TS_Handle;

static TS_StateTypeDef TS_State;

/******************************************************************************/
/*            Cortex-M7 Processor Interruption and Exception Handlers         */
/******************************************************************************/
void SysTick_Handler(void) {
    osSystickHandler();
}

/******************************************************************************/
/* STM32F7xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f7xx.s).                    */
/******************************************************************************/
void TIM8_TRG_COM_TIM14_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim14);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM14 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
  if (htim->Instance == TIM14) {
    HAL_IncTick();
  }
}

void EXTI15_10_IRQHandler(void) {
    HAL_GPIO_EXTI_IRQHandler(TS_INT_PIN);
}

/**
    @brief  EXTI line detection callbacks
    @param  GPIO_Pin: Specifies the pins connected EXTI line
    @retval None
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    static uint8_t TouchdOn = 0;

    if (GPIO_Pin == TS_INT_PIN) {
        /* Get the IT status register value */
        BSP_TS_GetState(&TS_State);

        if (TS_State.touchDetected) {
            if (TouchdOn < 4) {
                TouchdOn++;
            } else { /*TouchReleased */
                TouchdOn = 0;
                osSemaphoreRelease(xBinSem_TS_Handle);
            }
        }

        BSP_TS_ITClear();
    }
}