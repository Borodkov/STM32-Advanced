/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Src/stm32f7xx_it.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   Main Interrupt Service Routines.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_it.h"
#include "main.h"
#include "system_init.h"

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
    BenchmarkTick ++;
}


/******************************************************************************/
/*                 STM32F7xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f7xx.s).                                               */
/******************************************************************************/

/**
    @brief  This function handles DMA2 stream interrupt request.
    @param  None
    @retval None
*/
#ifdef CHECK_TRANSFER_DMA2
void DMA2_INSTANCE_IRQHANDLER(void) {
    /* Check the interrupt and clear flag */
    HAL_DMA_IRQHandler(&DmaHandle);
}
#endif


#ifdef CHECK_TRANSFER_DMA1_SPI
/**
    @brief  This function handles DMA1 SPIx Rx interrupt request.
    @param  None
    @retval None
*/
void SPIx_DMA_RX_IRQHandler(void) {
    HAL_DMA_IRQHandler(SpiHandle.hdmarx);
}

/**
    @brief  This function handles DMA1 SPIx Tx interrupt request.
    @param  None
    @retval None
*/
void SPIx_DMA_TX_IRQHandler(void) {
    HAL_DMA_IRQHandler(SpiHandle.hdmatx);
}

#endif
