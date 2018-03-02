/**
  ******************************************************************************
  * @file    app_usbFS.h
  * @author  PavelB
  * @version V1.0
  * @date    20-March-2017
  * @brief   virtual com port on USB FS
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_USBFS_H
#define __APP_USBFS_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "usbd_cdc_if.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define SEND_STRING_TO_USB()  if (AP.usbConnected) osSemaphoreRelease(xBinSem_USBHandle);
/* Exported functions ------------------------------------------------------- */
void usbFS_thread(void const * argument);

#ifdef __cplusplus
}
#endif

#endif /* __APP_USBFS_H */
