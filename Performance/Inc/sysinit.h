/**
  ******************************************************************************
  * @file    stm32f7_performances/Inc/sysinit.h 
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    24-February-2017 
  * @brief   Header for sysinit.c module
  ******************************************************************************
  */
  
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SYSINIT_H
#define __SYSINIT_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void system_init(void);
void Systick_reconfig(void);

#endif /* __SYSINIT_H */
