/**
  ******************************************************************************
  * @file    STM32F746_DSPDEMO\Inc\RTC.h
  * @author  MCD Application Team
  * @brief   Header for RTC.c module
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "global.h"
#include "dialog.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 
extern  RTC_HandleTypeDef RtcHandle;
/* Private function prototypes -----------------------------------------------*/
void     RTC_Init(void);
void     BACKUP_SaveParameter(uint32_t address, uint32_t data);
/* Private variables ---------------------------------------------------------*/
uint32_t BACKUP_RestoreParameter(uint32_t address);
#ifdef __cplusplus
}
#endif

#endif /*__RTC_H */
