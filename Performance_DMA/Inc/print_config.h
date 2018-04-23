/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Inc/print_config.h
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   Header for print_config.c module
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __PRINT_CONFIG_H
#define __PRINT_CONFIG_H

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
#define ON  "ON "
#define OFF "OFF"

/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Print_config(void);

#endif /* __PRINT_CONFIG_H */
