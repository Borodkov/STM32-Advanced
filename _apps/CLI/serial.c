/**
  ******************************************************************************
    File Name          : serial.c
    Description        : Code for serial driver for CLI UART application
  ******************************************************************************
*/

/* Standard includes. */
#include <stdlib.h>

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

/* Demo application includes. */
#include "serial.h"

/* STM32 includes. */
#include "stm32746g_discovery.h"

extern UART_HandleTypeDef huartCOM1Handler;

/* Misc. constants. */
#define serINVALID_QUEUE  ( ( QueueHandle_t ) 0 )
#define serNO_BLOCK   ( ( TickType_t ) 0 )

/* The queue used to hold received characters. */
static QueueHandle_t xRxedChars;
uint8_t aRxBuffer[1];
/* The queue used to hold characters waiting transmission. */
static QueueHandle_t xCharsForTx;

/*-----------------------------------------------------------*/

xComPortHandle xSerialPortInitMinimal(unsigned long ulWantedBaud, unsigned portBASE_TYPE uxQueueLength) {
    xComPortHandle xReturn;
    /* Create the queues used to hold Rx/Tx characters. */
    xRxedChars = xQueueCreate(uxQueueLength, (unsigned portBASE_TYPE) sizeof(signed char));
    xCharsForTx = xQueueCreate(uxQueueLength + 1, (unsigned portBASE_TYPE) sizeof(signed char));

    /*  If the queues were created correctly then setup the serial port
        hardware. */
    if ((xRxedChars != serINVALID_QUEUE) && (xCharsForTx != serINVALID_QUEUE)) {
        /* W8 character */
        HAL_UART_Receive_IT(&huartCOM1Handler, (uint8_t *)aRxBuffer, 1);
    } else {
        xReturn = (xComPortHandle) 0;
    }

    /*  This demo file only supports a single port but we have to return
        something to comply with the standard demo header file. */
    return xReturn;
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialGetChar(xComPortHandle pxPort, signed char *pcRxedChar, TickType_t xBlockTime) {
    /*  Get the next character from the buffer.  Return false if no characters
        are available, or arrive before xBlockTime expires. */
    if (xQueueReceive(xRxedChars, pcRxedChar, xBlockTime)) {
        return pdTRUE;
    } else {
        return pdFALSE;
    }
}
/*-----------------------------------------------------------*/

signed portBASE_TYPE xSerialPutChar(xComPortHandle pxPort, signed char cOutChar, TickType_t xBlockTime) {
    signed portBASE_TYPE xReturn;
    char cChar;

    if (xQueueSend(xCharsForTx, &cOutChar, xBlockTime) == pdPASS) {
        xReturn = pdPASS;

        if (xQueueReceive(xCharsForTx, &cChar, xBlockTime) == pdTRUE) {
            /* A character was retrieved from the queue so can be sent to the USART now. */
            HAL_UART_Transmit(&huartCOM1Handler, (uint8_t *)&cChar, 1, 1);
        }
    } else {
        xReturn = pdFAIL;
    }

    /* W8 character */
    HAL_UART_Receive_IT(&huartCOM1Handler, (uint8_t *)aRxBuffer, 1);
    return xReturn;
}
/*-----------------------------------------------------------*/

void vSerialPutString(xComPortHandle pxPort, const signed char *const pcString, unsigned short usStringLength) {
    signed char *pxNext;
    /* A couple of parameters that this port does not use. */
    (void) usStringLength;
    (void) pxPort;
    /*  NOTE: This implementation does not handle the queue being full as no
        block time is used! */
    /* The port handle is not required as this driver only supports UART1. */
    (void) pxPort;
    /* Send each character in the string, one at a time. */
    pxNext = (signed char *) pcString;

    while (*pxNext) {
        xSerialPutChar(pxPort, *pxNext, serNO_BLOCK);
        pxNext++;
    }
}
/*-----------------------------------------------------------*/

void vSerialClose(xComPortHandle xPort) {
    /* Not supported as not required by the demo application. */
}
/*-----------------------------------------------------------*/

/**
    @brief  Rx Transfer completed callback
    @param  UartHandle: UART handle
    @note   This example shows a simple way to report end of DMA Rx transfer, and
            you can add your own implementation.
    @retval None
*/
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle) {
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    char cChar;
    /*  A character has been received on the USART, send it to the Rx
        handler task. */
    cChar = (char)aRxBuffer[0];
    HAL_UART_Receive_IT(&huartCOM1Handler, (uint8_t *)aRxBuffer, 1);
    xQueueSendFromISR(xRxedChars, &cChar, &xHigherPriorityTaskWoken);
    /*  If sending or receiving from a queue has caused a task to unblock, and
        the unblocked task has a priority equal to or higher than the currently
        running task (the task this ISR interrupted), then xHigherPriorityTaskWoken
        will have automatically been set to pdTRUE within the queue send or receive
        function.  portEND_SWITCHING_ISR() will then ensure that this ISR returns
        directly to the higher priority unblocked task. */
    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

/**
    @brief  UART error callbacks
    @param  UartHandle: UART handle
    @note   This example shows a simple way to report transfer error, and you can
            add your own implementation.
    @retval None
*/
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle) {
    //Error_Handler();
}
