/**
  ******************************************************************************
    @file    STM32F746_DSPDEMO\Src\stm32f4xx_it.c
    @author  MCD Application Team
    @brief   Main Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "global.h"
#include "stm32f7xx_it.h"
#include "GUI.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
void DMA2_Stream0_IRQHandler(void);
void DACx_DMA_IRQHandler(void);
/* Private variables ---------------------------------------------------------*/
extern volatile GUI_TIMER_TIME OS_TimeMS;
extern LTDC_HandleTypeDef hltdc;
DAC_HandleTypeDef    DacHandleDspDemo;
ADC_HandleTypeDef    AdcHandleDspDemo;
TIM_HandleTypeDef    Tim3HandleDspDemo;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M7 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
    @brief  This function handles NMI exception.
    @param  None
    @retval None
*/
void NMI_Handler(void)
{}

/**
    @brief  This function handles Hard Fault exception.
    @param  None
    @retval None
*/
void HardFault_Handler(void) {
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {}
}

/**
    @brief  This function handles Memory Manage exception.
    @param  None
    @retval None
*/
void MemManage_Handler(void) {
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {}
}

/**
    @brief  This function handles Bus Fault exception.
    @param  None
    @retval None
*/
void BusFault_Handler(void) {
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {}
}

/**
    @brief  This function handles Usage Fault exception.
    @param  None
    @retval None
*/
void UsageFault_Handler(void) {
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {}
}

/**
    @brief  This function handles Debug Monitor exception.
    @param  None
    @retval None
*/
void DebugMon_Handler(void)
{}

/**
    @brief  This function handles SysTick Handler.
    @param  None
    @retval None
*/
void SysTick_Handler(void) {
    /* Update the LocalTime by adding 1 ms each SysTick interrupt */
    HAL_IncTick();
    OS_TimeMS++;
}

/******************************************************************************/
/*                 STM32F7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f7xx.s).                                               */
/******************************************************************************/

/**
    @brief  This function handles DMA interrupt request.
    @param  None
    @retval None
*/
void DACx_DMA_IRQHandler(void) {
    HAL_DMA_IRQHandler(DacHandleDspDemo.DMA_Handle1);
}
/**
    @brief  This function handles DMA interrupt request.
    @param  None
    @retval None
*/
void DMA2_Stream0_IRQHandler(void) {
    HAL_DMA_IRQHandler(AdcHandleDspDemo.DMA_Handle);
}

/**
    @brief  This function handles TIM interrupt request.
    @param  None
    @retval None
*/
void TIM3_IRQHandler(void) {
    HAL_TIM_IRQHandler(&Tim3HandleDspDemo);
}

/**
    @brief  This function handles LTDC global interrupt request.
    @param  None
    @retval None
*/
void LTDC_IRQHandler(void) {
    HAL_LTDC_IRQHandler(&hltdc);
}
