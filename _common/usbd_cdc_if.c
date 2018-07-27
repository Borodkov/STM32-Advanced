/*******************************************************************************
    Description: 
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"
#include "main.h"
#include "cmsis_os.h"

/** @defgroup USBD_CDC_Private_Defines
    @{
*/
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE  64
#define APP_TX_DATA_SIZE  64

/** @defgroup USBD_CDC_Private_Variables
    @{
*/
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/* Received Data over USB are stored in this buffer       */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/* Send Data over USB CDC are stored in this buffer       */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

extern apSettings_t AP;
extern osSemaphoreId xBinSem_USBHandle;

/** @defgroup USBD_CDC_IF_Exported_Variables
    @{
*/
extern USBD_HandleTypeDef hUsbDeviceFS;

/** @defgroup USBD_CDC_Private_FunctionPrototypes
    @{
*/
static int8_t CDC_Init_FS(void);
static int8_t CDC_DeInit_FS(void);
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length);
static int8_t CDC_Receive_FS(uint8_t *pbuf, uint32_t *Len);

USBD_CDC_ItfTypeDef USBD_Interface_fops_FS = {
    CDC_Init_FS,
    CDC_DeInit_FS,
    CDC_Control_FS,
    CDC_Receive_FS
};

/* Private functions ---------------------------------------------------------*/
/**
    @brief  CDC_Init_FS
            Initializes the CDC media low layer over the FS USB IP
    @param  None
    @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
*/
static int8_t CDC_Init_FS(void) {
    /* Set Application Buffers */
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, UserTxBufferFS, 0);
    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, UserRxBufferFS);
    return (USBD_OK);
}

/**
    @brief  CDC_DeInit_FS
            DeInitializes the CDC media low layer
    @param  None
    @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
*/
static int8_t CDC_DeInit_FS(void) {
    return (USBD_OK);
}

/**
    @brief  CDC_Control_FS
            Manage the CDC class requests
    @param  cmd: Command code
    @param  pbuf: Buffer containing command data (request parameters)
    @param  length: Number of data to be sent (in bytes)
    @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
*/
static int8_t CDC_Control_FS(uint8_t cmd, uint8_t *pbuf, uint16_t length) {
    switch (cmd) {
        case CDC_SEND_ENCAPSULATED_COMMAND:
            break;

        case CDC_GET_ENCAPSULATED_RESPONSE:
            break;

        case CDC_SET_COMM_FEATURE:
            break;

        case CDC_GET_COMM_FEATURE:
            break;

        case CDC_CLEAR_COMM_FEATURE:
            break;

        /*******************************************************************************/
        /* Line Coding Structure                                                       */
        /*-----------------------------------------------------------------------------*/
        /* Offset | Field       | Size | Value  | Description                          */
        /* 0      | dwDTERate   |   4  | Number |Data terminal rate, in bits per second*/
        /* 4      | bCharFormat |   1  | Number | Stop bits                            */
        /*                                        0 - 1 Stop bit                       */
        /*                                        1 - 1.5 Stop bits                    */
        /*                                        2 - 2 Stop bits                      */
        /* 5      | bParityType |  1   | Number | Parity                               */
        /*                                        0 - None                             */
        /*                                        1 - Odd                              */
        /*                                        2 - Even                             */
        /*                                        3 - Mark                             */
        /*                                        4 - Space                            */
        /* 6      | bDataBits  |   1   | Number Data bits (5, 6, 7, 8 or 16).          */
        /*******************************************************************************/
        case CDC_SET_LINE_CODING:
            break;

        case CDC_GET_LINE_CODING:
            break;

        case CDC_SET_CONTROL_LINE_STATE:
            break;

        case CDC_SEND_BREAK:
            break;

        default:
            break;
    }

    return (USBD_OK);
}

/**
    @brief  CDC_Receive_FS
            Data received over USB OUT endpoint are sent over CDC interface
            through this function.

            @note
            This function will block any OUT packet reception on USB endpoint
            untill exiting this function. If you exit this function before transfer
            is complete on CDC interface (ie. using DMA controller) it will result
            in receiving more data while previous ones are still not sent.

    @param  Buf: Buffer of data to be received
    @param  Len: Number of data received (in bytes)
    @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL
*/
static int8_t CDC_Receive_FS(uint8_t *Buf, uint32_t *Len) {
    static uint32_t i = 0;

    if (AP.usbConnected) {
        if (*Buf == '\r') {
            // clear buf
            for (; i < 256; i++) { AP.buffers.STRING[i] = 0; }

            i = 0;
            CDC_Transmit_FS("\r\n", 2);
            osSemaphoreRelease(xBinSem_USBHandle);
        } else {
            AP.buffers.STRING[i] = *Buf;

            if (i < 256) {
                i++;
            } else {
                i = 0;
            }
        }

        CDC_Transmit_FS(Buf, 1);
    } else {
        AP.usbConnected = 1;
    }

    USBD_CDC_SetRxBuffer(&hUsbDeviceFS, &Buf[0]);
    USBD_CDC_ReceivePacket(&hUsbDeviceFS);
    return (USBD_OK);
}

/**
    @brief  CDC_Transmit_FS
            Data send over USB IN endpoint are sent over CDC interface
            through this function.

    @param  Buf: Buffer of data to be send
    @param  Len: Number of data to be send (in bytes)
    @retval Result of the operation: USBD_OK if all operations are OK else USBD_FAIL or USBD_BUSY
*/
uint8_t CDC_Transmit_FS(uint8_t *Buf, uint16_t Len) {
    uint8_t result = USBD_OK;
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;

    if (hcdc->TxState != 0) {
        return USBD_BUSY;
    }

    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
    result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
    return result;
}
