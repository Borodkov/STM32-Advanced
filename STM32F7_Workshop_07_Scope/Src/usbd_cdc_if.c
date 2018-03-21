/**
  ******************************************************************************
    @file           : usbd_cdc_if.c
    @brief          :
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"
#include "scope.h"

/** @defgroup USBD_CDC_Private_Defines
    @{
*/
/* Define size for the receive and transmit buffer over CDC */
/* It's up to user to redefine and/or remove those define */
#define APP_RX_DATA_SIZE  4
#define APP_TX_DATA_SIZE  4

/** @defgroup USBD_CDC_Private_Variables
    @{
*/
/* Create buffer for reception and transmission           */
/* It's up to user to redefine and/or remove those define */
/* Received Data over USB are stored in this buffer       */
uint8_t UserRxBufferFS[APP_RX_DATA_SIZE];

/* Send Data over USB CDC are stored in this buffer       */
uint8_t UserTxBufferFS[APP_TX_DATA_SIZE];

extern TIM_HandleTypeDef htim2;//handle from UART3
extern OSC_TypeDef oscConfig;
uint8_t tempbuf[7];//buffer where we styore information about line coding, we must simulate VCP otherwaise Wi9ndows stop communicate with us.

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
            /*  Set line coding handling we gen information about coding from Windows,
                if we not store this value and on get line coding send it back the Windows will stop communication*/
            tempbuf[0] = pbuf[0];
            tempbuf[1] = pbuf[1];
            tempbuf[2] = pbuf[2];
            tempbuf[3] = pbuf[3];
            tempbuf[4] = pbuf[4];
            tempbuf[5] = pbuf[5];
            tempbuf[6] = pbuf[6];
            break;

        case CDC_GET_LINE_CODING:
            /*Get line coding handling, send information about coding back to Windows*/
            pbuf[0] = tempbuf[0];
            pbuf[1] = tempbuf[1];
            pbuf[2] = tempbuf[2];
            pbuf[3] = tempbuf[3];
            pbuf[4] = tempbuf[4];
            pbuf[5] = tempbuf[5];
            pbuf[6] = tempbuf[6];
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
    /** Expected buffer format:
        Byte[0] .. message type:
        0x1 - trigger level set
        Byte[2..3] trigger values, little endian format
        0x2 - sample rate set
        Byte[1..3] sample rate value, little endian format
        0x3 - number of samples set
        Byte[1..3] number of samples value, little endian format
        0x4 - trigger edge set
        Byte[3] trigger edge set
        0x5 - trigger mode set
        Byte[3] trigger mode set, used first 2-bites
    */

    /*correct message length check*/
    if (*Len == 4) {
        /*0x1 - trigger level set*/
        if (Buf[0] == 0x1) {
            oscConfig.triggerLevel = (Buf[2] & 0xFF) | ((Buf[3] << 8) & 0xFF00);
        }
        /*0x2 - sample rate set*/
        else if (Buf[0] == 0x2) {
            uint32_t sampleRate = (Buf[1] & 0xFF) | ((Buf[2] << 8) & 0xFF00) | ((Buf[3] << 16) & 0xFF0000);
            uint32_t x = 100000000 / sampleRate;//100MHz is base TIM2 frequency
            uint32_t prescaler = 1;
            htim2.Init.Prescaler = 0;

            while ((x / prescaler) > 65536) {
                prescaler++;
            }

            x /= prescaler;
            htim2.Init.Period = x - 1;
            htim2.Init.Prescaler = prescaler - 1;
            TIM_Base_SetConfig(htim2.Instance, &htim2.Init);
        }
        /*0x3 - number of samples set*/
        else if (Buf[0] == 0x3) {
            oscConfig.newNumberOfSamples = (Buf[1] & 0xFF) | ((Buf[2] << 8) & 0xFF00) | ((Buf[3] << 16) & 0xFF0000);
        }
        /*0x4 - trigger edge set*/
        else if (Buf[0] == 0x4) {
            oscConfig.triggerEdge = Buf[3];
        }
        /*0x5 - trigger mode set*/
        else if (Buf[0] == 0x5) {
            oscConfig.triggerMode = Buf[3] & 0x3;
        }
    }

    /*Prepare USB to receive more data*/
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
    /* USER CODE BEGIN 7 */
    USBD_CDC_SetTxBuffer(&hUsbDeviceFS, Buf, Len);
    result = USBD_CDC_TransmitPacket(&hUsbDeviceFS);
    /* USER CODE END 7 */
    return result;
}
