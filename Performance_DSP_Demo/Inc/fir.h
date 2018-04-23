/**
  ******************************************************************************
  * @file    STM32F746_DSPDEMO\Inc\fir.h 
  * @author  MCD Application Team
  * @brief   Header for fir_processing.c module
  ******************************************************************************
  */

#ifndef __FIR_H
#define __FIR_H
/* Includes ------------------------------------------------------------------*/
#include "global.h"
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Exported Defines ------------------------------------------------------- */
#define TEST_LENGTH_SAMPLES             240
#define BLOCK_SIZE                      32 
#define NUM_TAPS                        29 
#define NUM_FIR_TAPS_Q15                56 
#define BLOCKSIZE                       32
/* Exported functions ------------------------------------------------------- */
void FIR_PROCESSING_F32Process(void);
void FIR_PROCESSING_Q31Process(void);
void FIR_PROCESSING_Q15Process(int LP_or_HP);

#endif /* __FIR_H */
