/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Inc/serial.h
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   Header for serial.c module
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SERIAL_H
#define __SERIAL_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#if defined ( PRINTF_UART )
/*  User can use this section to tailor USARTx/UARTx instance used and associated
    resources */
/* Definition for USARTx clock resources */
#define USARTx                           USART1
#define USARTx_CLK_ENABLE()              __USART1_CLK_ENABLE()
#define USARTx_TX_GPIO_CLK_ENABLE()      __GPIOA_CLK_ENABLE()

#define USARTx_FORCE_RESET()             __USART1_FORCE_RESET()
#define USARTx_RELEASE_RESET()           __USART1_RELEASE_RESET()

/* Definition for USARTx Pins */
#define USARTx_TX_PIN                    GPIO_PIN_9
#define USARTx_TX_GPIO_PORT              GPIOA
#define USARTx_TX_AF                     GPIO_AF7_USART1

#endif /* defined ( PRINTF_UART ) */

/* Exported functions ------------------------------------------------------- */
void Printf_Init(void);

#endif /* __SERIAL_H */
