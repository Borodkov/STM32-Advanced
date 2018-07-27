/*******************************************************************************
    Description: 
*******************************************************************************/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBH_CONF__H__
#define __USBH_CONF__H__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "stm32f7xx_hal.h"

/**
    MiddleWare name : USB_HOST
    MiddleWare fileName : usbh_conf.h
    MiddleWare version :
*/
/*----------   -----------*/
#define USBH_MAX_NUM_ENDPOINTS      2

/*----------   -----------*/
#define USBH_MAX_NUM_INTERFACES      2

/*----------   -----------*/
#define USBH_MAX_NUM_CONFIGURATION      1

/*----------   -----------*/
#define USBH_KEEP_CFG_DESCRIPTOR      1

/*----------   -----------*/
#define USBH_MAX_NUM_SUPPORTED_CLASS      1

/*----------   -----------*/
#define USBH_MAX_SIZE_CONFIGURATION      256

/*----------   -----------*/
#define USBH_MAX_DATA_BUFFER      512

/*----------   -----------*/
#define USBH_DEBUG_LEVEL      0

/*----------   -----------*/
#define USBH_USE_OS      1

/****************************************/
/* #define for FS and HS identification */
#define HOST_HS         0
#define HOST_FS         1

/** @defgroup USBH_Exported_Macros
    @{
*/
#if (USBH_USE_OS == 1)
#include "cmsis_os.h"
#define   USBH_PROCESS_PRIO          osPriorityNormal
#define   USBH_PROCESS_STACK_SIZE    ((uint16_t)128)
#endif

/* Memory management macros */
#define USBH_malloc               malloc
#define USBH_free                 free
#define USBH_memset               memset
#define USBH_memcpy               memcpy

/* DEBUG macros */

#if (USBH_DEBUG_LEVEL > 0)
#define  USBH_UsrLog(...)   printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBH_UsrLog(...)
#endif

#if (USBH_DEBUG_LEVEL > 1)

#define  USBH_ErrLog(...)   printf("ERROR: ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBH_ErrLog(...)
#endif


#if (USBH_DEBUG_LEVEL > 2)
#define  USBH_DbgLog(...)   printf("DEBUG : ") ;\
                            printf(__VA_ARGS__);\
                            printf("\n");
#else
#define USBH_DbgLog(...)
#endif

#ifdef __cplusplus
}
#endif

#endif /* __USBH_CONF__H__ */
