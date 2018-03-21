/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery_qspi.h"

#define DEBUG_PIN_HIGH  GPIOI->ODR |= GPIO_PIN_3
#define DEBUG_PIN_LOW  GPIOI->ODR &= ~GPIO_PIN_3

#define TIM_MEASURE_START time_start = __HAL_TIM_GET_COUNTER(&TimHandle); \
                                       DEBUG_PIN_HIGH
#define TIM_MEASURE_END time_end = __HAL_TIM_GET_COUNTER(&TimHandle);     \
                        time_diff = time_end - time_start;                \
                        DEBUG_PIN_LOW

#define LCD_X_SIZE  480
#define LCD_Y_SIZE  272

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __MAIN_H */
