/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"
#include "stm32746g_discovery_audio.h"
#include "TEXT.h"
#include "GUI.h"
#include "osc.h"

/* Exported types ------------------------------------------------------------*/
typedef enum {FAILED = 0, PASSED = !FAILED} TestStatus;

/* Exported constants --------------------------------------------------------*/
#define DEBUG_PIN_HIGH  GPIOI->ODR |= GPIO_PIN_3
#define DEBUG_PIN_LOW  GPIOI->ODR &= ~GPIO_PIN_3

#define YOU_CAN_PUT_BREAKPOINT_HERE __NOP()

//#define TIM_MEASURE_START time_start = SysTick->VAL
//#define TIM_MEASURE_END time_end = SysTick->VAL;            \
//                       time_diff = time_start - time_end

#define TIM_MEASURE_START time_start = hTim2.Instance->CNT;
#define TIM_MEASURE_END time_end = hTim2.Instance->CNT;            \
                       time_diff = time_end - time_start

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */
