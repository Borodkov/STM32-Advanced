/**
 ******************************************************************************
    @file            : USB_HOST
    @version         : v1.0_Cube
    @brief           : Header for usb_host file.
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __usb_host_H
#define __usb_host_H
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

typedef enum {
    APPLICATION_IDLE = 0,
    APPLICATION_START,
    APPLICATION_READY,
    APPLICATION_DISCONNECT,
} ApplicationTypeDef;

void MX_USB_HOST_Init(void);

#ifdef __cplusplus
}
#endif
#endif /*__usb_host_H */
