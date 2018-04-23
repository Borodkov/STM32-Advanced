/**
  ******************************************************************************
  * @file    stm32f7_performances/Inc/signal_input.h 
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    24-February-2017 
  * @brief   Header for signal_input.c module
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SIGNAL_INPUT_H
#define __SIGNAL_INPUT_H

/* Includes ------------------------------------------------------------------*/
#include "arm_math.h" /* float32_t */

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define TEST_LENGTH_SAMPLES   2048
extern float32_t aSignalInput_f32_10khz[TEST_LENGTH_SAMPLES];

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __SIGNAL_INPUT_H */
