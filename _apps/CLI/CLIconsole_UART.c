/**
  ******************************************************************************
    File Name          : CLIconsole_UART.c
    Description        : Code for serial driver for CLI UART application
  ******************************************************************************
*/

/* Standard includes. */
#include "string.h"
#include "stdio.h"

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

/* Example includes. */
#include "FreeRTOS_CLI.h"

///* Demo application includes. */
#include "serial.h"

#define cmdQUEUE_LENGTH 25

/* DEL acts as a backspace. */
#define cmdASCII_DEL (0x7F)

#define cmdMAX_MUTEX_WAIT (((TickType_t)300) / (portTICK_PERIOD_MS))

#ifndef configCLI_BAUD_RATE
#define configCLI_BAUD_RATE 115200
#endif

/*-----------------------------------------------------------*/

/*
    The task that implements the command console processing.
*/
static void prvUARTCommandConsoleTask(void *pvParameters);
void vUARTCommandConsoleStart(uint16_t usStackSize, UBaseType_t uxPriority);

SemaphoreHandle_t xTxMutex = NULL;
static xComPortHandle xPort = 0;

/* Serial drivers that use task notifications may need the CLI task's handle. */
TaskHandle_t xCLITaskHandle = NULL;

/*-----------------------------------------------------------*/

void vUARTCommandConsoleStart(uint16_t usStackSize, UBaseType_t uxPriority) {
    /* Create the semaphore used to access the UART Tx. */
    xTxMutex = xSemaphoreCreateMutex();
    configASSERT(xTxMutex);
    /* Create that task that handles the console itself. */
    xTaskCreate(prvUARTCommandConsoleTask, /* The task that implements the command console. */
                "CLI",                     /* Text name assigned to the task.  This is just to assist debugging.  The kernel does not use this name itself. */
                usStackSize,               /* The size of the stack allocated to the task. */
                NULL,                      /* The parameter is not used, so NULL is passed. */
                uxPriority,                /* The priority allocated to the task. */
                &xCLITaskHandle);          /* Serial drivers that use task notifications may need the CLI task's handle. */
}
/*-----------------------------------------------------------*/

static void prvUARTCommandConsoleTask(void *pvParameters) {
    char cRxedChar;
    uint8_t ucInputIndex = 0;
    char *pcOutputString;
    static char cInputString[cmdMAX_INPUT_SIZE], cLastInputString[cmdMAX_INPUT_SIZE];
    BaseType_t xReturned;
    xComPortHandle xPort;
    (void)pvParameters;
    /*  Obtain the address of the output buffer.  Note there is no mutual
        exclusion on this buffer as it is assumed only one command console interface
        will be used at any one time. */
    pcOutputString = FreeRTOS_CLIGetOutputBuffer();
    /* Initialise the UART. */
    xPort = xSerialPortInitMinimal(configCLI_BAUD_RATE, cmdQUEUE_LENGTH);
    /* Send the welcome message. */
    vSerialPutString(xPort, (signed char *)pcWelcomeMessage, strlen(pcWelcomeMessage));

    for (;;) {
        /*  Wait for the next character.  The while loop is used in case
            INCLUDE_vTaskSuspend is not set to 1 - in which case portMAX_DELAY will
            be a genuine block time rather than an infinite block time. */
        while (xSerialGetChar(xPort, (signed char *)&cRxedChar, portMAX_DELAY) != pdPASS)
            ;

        /* Ensure exclusive access to the UART Tx. */
        if (xSemaphoreTake(xTxMutex, cmdMAX_MUTEX_WAIT) == pdPASS) {
            /* Echo the character back. */
            xSerialPutChar(xPort, cRxedChar, portMAX_DELAY);

            /* Was it the end of the line? */
            if ((cRxedChar == '\n') || (cRxedChar == '\r')) {
                /* Just to space the output from the input. */
                vSerialPutString(xPort, (signed char *)pcNewLine, strlen(pcNewLine));

                /*  See if the command is empty, indicating that the last command
                    is to be executed again. */
                if (ucInputIndex == 0) {
                    /* Copy the last command back into the input string. */
                    strcpy(cInputString, cLastInputString);
                }

                /*  Pass the received command to the command interpreter.  The
                    command interpreter is called repeatedly until it returns
                    pdFALSE (indicating there is no more output) as it might
                    generate more than one string. */
                do {
                    /* Get the next output string from the command interpreter. */
                    xReturned = FreeRTOS_CLIProcessCommand(cInputString, pcOutputString, configCOMMAND_INT_MAX_OUTPUT_SIZE);
                    /* Write the generated string to the UART. */
                    vSerialPutString(xPort, (signed char *)pcOutputString, strlen(pcOutputString));
                } while (xReturned != pdFALSE);

                /*  All the strings generated by the input command have been
                    sent.  Clear the input string ready to receive the next command.
                    Remember the command that was just processed first in case it is
                    to be processed again. */
                strcpy(cLastInputString, cInputString);
                ucInputIndex = 0;
                memset(cInputString, 0x00, cmdMAX_INPUT_SIZE);
                vSerialPutString(xPort, (signed char *)pcEndOfOutputMessage, strlen(pcEndOfOutputMessage));
            } else {
                if (cRxedChar == '\r') {
                    /* Ignore the character. */
                } else if ((cRxedChar == '\b') || (cRxedChar == cmdASCII_DEL)) {
                    /*  Backspace was pressed.  Erase the last character in the
                        string - if any. */
                    if (ucInputIndex > 0) {
                        ucInputIndex--;
                        cInputString[ucInputIndex] = '\0';
                    }
                } else {
                    /*  A character was entered.  Add it to the string entered so
                        far.  When a \n is entered the complete string will be
                        passed to the command interpreter. */
                    if ((cRxedChar >= ' ') && (cRxedChar <= '~')) {
                        if (ucInputIndex < cmdMAX_INPUT_SIZE) {
                            cInputString[ucInputIndex] = cRxedChar;
                            ucInputIndex++;
                        }
                    }
                }
            }

            /* Must ensure to give the mutex back. */
            xSemaphoreGive(xTxMutex);
        }
    }
}
/*-----------------------------------------------------------*/

void vOutputString(const char *const pcMessage) {
    if (xSemaphoreTake(xTxMutex, cmdMAX_MUTEX_WAIT) == pdPASS) {
        vSerialPutString(xPort, (signed char *)pcMessage, strlen(pcMessage));
        xSemaphoreGive(xTxMutex);
    }
}
/*-----------------------------------------------------------*/

void vOutputChar(const char cChar, const TickType_t xTicksToWait) {
    if (xSemaphoreTake(xTxMutex, xTicksToWait) == pdPASS) {
        xSerialPutChar(xPort, cChar, xTicksToWait);
        xSemaphoreGive(xTxMutex);
    }
}
/*-----------------------------------------------------------*/
