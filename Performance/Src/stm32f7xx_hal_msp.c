/**
  ******************************************************************************
    @file    stm32f7xx_hal_msp.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   HAL MSP module.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "serial.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
    @brief  Initializes the Global MSP.
    @param  None
    @retval None
*/
void HAL_MspInit(void) {
    /*  NOTE : This function is generated automatically by STM32CubeMX and eventually
              modified by the user
    */
}

/**
    @brief  DeInitializes the Global MSP.
    @param  None
    @retval None
*/
void HAL_MspDeInit(void) {
    /*  NOTE : This function is generated automatically by STM32CubeMX and eventually
              modified by the user
    */
}

#if defined ( PRINTF_UART )
/**
    @brief UART MSP Initialization
           This function configures the hardware resources used in this example:
              - Peripheral's clock enable
              - Peripheral's GPIO Configuration
    @param huart: UART handle pointer
    @retval None
*/
void HAL_UART_MspInit(UART_HandleTypeDef *huart) {
    GPIO_InitTypeDef  GPIO_InitStruct;
    RCC_PeriphCLKInitTypeDef RCC_PeriphClkInit;
    /*##-1- Enable peripherals and GPIO Clocks #################################*/
    /* Enable GPIO TX clock */
    USARTx_TX_GPIO_CLK_ENABLE();
    /* Select SysClk as source of USART1 clocks */
    RCC_PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
    RCC_PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_SYSCLK;
    HAL_RCCEx_PeriphCLKConfig(&RCC_PeriphClkInit);
    /* Enable USARTx clock */
    USARTx_CLK_ENABLE();
    /*##-2- Configure peripheral GPIO ##########################################*/
    /* UART TX GPIO pin configuration  */
    GPIO_InitStruct.Pin       = USARTx_TX_PIN;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = USARTx_TX_AF;
    HAL_GPIO_Init(USARTx_TX_GPIO_PORT, &GPIO_InitStruct);
}

/**
    @brief UART MSP De-Initialization
           This function frees the hardware resources used in this example:
             - Disable the Peripheral's clock
             - Revert GPIO and NVIC configuration to their default state
    @param huart: UART handle pointer
    @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart) {
    /*##-1- Reset peripherals ##################################################*/
    USARTx_FORCE_RESET();
    USARTx_RELEASE_RESET();
    /*##-2- Disable peripherals and GPIO Clocks #################################*/
    /* Configure UART Tx as alternate function  */
    HAL_GPIO_DeInit(USARTx_TX_GPIO_PORT, USARTx_TX_PIN);
}

#endif /* defined ( PRINTF_UART ) */
