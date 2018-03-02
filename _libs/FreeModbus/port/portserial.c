/* ----------------------- Modbus includes ----------------------------------*/
#include "mb.h"

/* ----------------------- Bard specific ----------------------------------*/
extern UART_HandleTypeDef huart1;

/* ----------------------- Static variables ---------------------------------*/
UCHAR ucGIEWasEnabled = FALSE;
UCHAR ucCriticalNesting = 0x00;

/* ----------------------- Start implementation -----------------------------*/
void
vMBPortSerialEnable( BOOL xRxEnable, BOOL xTxEnable )
{
  /* If xRXEnable enable serial receive interrupts. If xTxENable enable
   * transmitter empty interrupts.
   */
  if(xRxEnable)
  {
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE);
  }
  else
  {
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_RXNE);
  }
  
  if(xTxEnable)
  {
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_TXE);
  }
  else
  {
    __HAL_UART_DISABLE_IT(&huart1, UART_IT_TXE);
  }
}

BOOL
xMBPortSerialInit( UCHAR ucPort, ULONG ulBaudRate, UCHAR ucDataBits, eMBParity eParity )
{
  BOOL    bInitialized = TRUE;
  
  //
  // ...
  //
  
  return bInitialized;
}

BOOL
xMBPortSerialPutByte( CHAR ucByte )
{
  /* Put a byte in the UARTs transmit buffer. This function is called
   * by the protocol stack if pxMBFrameCBTransmitterEmpty( ) has been
   * called. */
    huart1.Instance->TDR=ucByte;
    return TRUE;
}

BOOL
xMBPortSerialGetByte( CHAR * pucByte )
{
  /* Return the byte in the UARTs receive buffer. This function is called
   * by the protocol stack after pxMBFrameCBByteReceived( ) has been called.
   */
  if(huart1.Init.Parity == UART_PARITY_NONE)
  {
    *pucByte = (uint8_t)(huart1.Instance->RDR & (uint8_t)0x00FF);
  }
  else
  {
    *pucByte = (uint8_t)(huart1.Instance->RDR & (uint8_t)0x007F);
  }
  return TRUE;
}

void MODBUS_UART_IRQHandler(UART_HandleTypeDef *huart)
{
  if(__HAL_UART_GET_FLAG(huart,UART_FLAG_TXE) && __HAL_UART_GET_IT_SOURCE(huart,UART_IT_TXE))
  {
    pxMBFrameCBTransmitterEmpty(  );
  }
  else if(__HAL_UART_GET_FLAG(huart,UART_FLAG_RXNE) && __HAL_UART_GET_IT_SOURCE(huart,UART_IT_RXNE))
  {
    pxMBFrameCBByteReceived(  );
  }
}
