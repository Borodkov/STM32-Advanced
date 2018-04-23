/**
  ******************************************************************************
  * @file    STM32F746_DSPDEMO\Inc\calibration.h 
  * @author  MCD Application Team
  * @brief   Header for CALIBRATION.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __CALIBRATION_H
#define __CALIBRATION_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "global.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void CALIBRATION_Check(void);
/* Private variables ---------------------------------------------------------*/
uint8_t  CALIBRATION_IsDone(void);
uint16_t CALIBRATION_GetX(uint16_t x);
uint16_t CALIBRATION_GetY(uint16_t y);

#ifdef __cplusplus
}
#endif

#endif /*__CALIBRATION_H */
