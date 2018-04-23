/**
  ******************************************************************************
  * @file    STM32F746_DSPDEMO\Inc\fft.h 
  * @author  MCD Application Team
  * @brief   Header for fft_processing.c module
  ******************************************************************************
  */


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FFT_H
#define __FFT_H
/* Includes ------------------------------------------------------------------*/
#include "global.h"
#define FFT_Length_Tab 2048

extern int FFT_SIZE_CHOOSE;

/* Private functions ---------------------------------------------------------*/
void FFT_PROCESSING_F32Process(uint32_t FFT_Length);
void FFT_PROCESSING_Q15Process(uint32_t FFT_Length);
void FFT_PROCESSING_Q31Process(uint32_t FFT_Length);

#endif /* __FFT_H */
