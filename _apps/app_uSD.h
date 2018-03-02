/**
  ******************************************************************************
  * @file    app_uSD.h
  * @author  PavelB
  * @version V1.0
  * @date    20-March-2017
  * @brief   uSD card
  ******************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __APP_USD_H
#define __APP_USD_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define SEND_STRING_TO_uSD()  if (AP.uSDConnected) osSemaphoreRelease(xBinSem_uSDHandle);
/* Exported functions ------------------------------------------------------- */
void uSD_thread(void const * argument);

#ifdef __cplusplus
}
#endif

#endif /* __APP_USD_H */
