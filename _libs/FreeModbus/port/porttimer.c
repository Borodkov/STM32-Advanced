/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"

/* ----------------------- Bard specific ----------------------------------*/
TIM_HandleTypeDef htim10;

/* ----------------------- Static variables ---------------------------------*/
static uint16_t timeout = 0;
static uint16_t counter = 0;

/* ----------------------- Start implementation -----------------------------*/
BOOL
xMBPortTimersInit( USHORT usTim1Timeout50us )
{  
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = (uint32_t) (SystemCoreClock / 1000000) - 1;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 50-1; // 50 us
  
  timeout = usTim1Timeout50us;
  
  return HAL_OK == HAL_TIM_Base_Init(&htim10) ? TRUE : FALSE;
}

void
vMBPortTimersEnable( void )
{
  /* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
  counter = 0;
  
  HAL_TIM_Base_Start_IT(&htim10);
}

void
vMBPortTimersDisable( void )
{
  HAL_TIM_Base_Stop_IT(&htim10);
}

/* Create an ISR which is called whenever the timer has expired. This function
 * must then call pxMBPortCBTimerExpired( ) to notify the protocol stack that
 * the timer has expired.
 */
static void prvvTIMERExpiredISR( void )
{
    ( void )pxMBPortCBTimerExpired(  );
}

/**
* @brief This function handles TIM6 global interrupt, DAC1 and DAC2 underrun error interrupts.
*/
void TIM1_UP_TIM10_IRQHandler(void)
{
  __HAL_TIM_CLEAR_IT(&htim10,TIM_IT_UPDATE);
  
  /* TIM Update event */
  if (++counter >= timeout)
     prvvTIMERExpiredISR();
}