/**
  ******************************************************************************
    @file    stm32f7_performances/stm32f7xx_it.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   Main Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_it.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t BenchmarkTick = 0;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
    @brief   This function handles NMI exception.
    @param  None
    @retval None
*/
void NMI_Handler(void) {
}

/**
    @brief  This function handles Hard Fault exception.
    @param  None
    @retval None
*/
void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1) {
    }
}

/**
    @brief  This function handles Memory Manage exception.
    @param  None
    @retval None
*/
void MemManage_Handler(void) {
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1) {
    }
}

/**
    @brief  This function handles Bus Fault exception.
    @param  None
    @retval None
*/
void BusFault_Handler(void) {
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1) {
    }
}

/**
    @brief  This function handles Usage Fault exception.
    @param  None
    @retval None
*/
void UsageFault_Handler(void) {
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1) {
    }
}

/**
    @brief  This function handles SVCall exception.
    @param  None
    @retval None
*/
void SVC_Handler(void) {
}

/**
    @brief  This function handles Debug Monitor exception.
    @param  None
    @retval None
*/
void DebugMon_Handler(void) {
}

/**
    @brief  This function handles PendSVC exception.
    @param  None
    @retval None
*/
void PendSV_Handler(void) {
}

/**
    @brief  This function handles SysTick Handler.
    @param  None
    @retval None
*/
void SysTick_Handler(void) {
    HAL_IncTick();
    BenchmarkTick++;
}
