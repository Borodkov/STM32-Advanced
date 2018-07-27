/*******************************************************************************
    Description:
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_host.h"
#include "usbh_core.h"
#include "usbh_msc.h"

/* USB Host Core handle declaration */
USBH_HandleTypeDef hUsbHostHS;
ApplicationTypeDef Appli_state = APPLICATION_IDLE;

/*
    user callbak declaration
*/
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id);

/* init function */
void MX_USB_HOST_Init(void) {
    /* Init Host Library,Add Supported Class and Start the library*/
    USBH_Init(&hUsbHostHS, USBH_UserProcess, HOST_HS);
    USBH_RegisterClass(&hUsbHostHS, USBH_MSC_CLASS);
    USBH_Start(&hUsbHostHS);
}

/*
    user callback definition
*/
static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id) {
    switch (id) {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            Appli_state = APPLICATION_DISCONNECT;
            break;

        case HOST_USER_CLASS_ACTIVE:
            Appli_state = APPLICATION_READY;
            break;

        case HOST_USER_CONNECTION:
            Appli_state = APPLICATION_START;
            break;

        default:
            break;
    }
}
