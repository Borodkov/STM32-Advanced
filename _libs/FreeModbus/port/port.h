#ifndef _PORT_H
#define _PORT_H

/* ----------------------- Platform includes --------------------------------*/
#include "stm32746g_discovery.h"

/* ----------------------- Defines ------------------------------------------*/
#define	INLINE
#define PR_BEGIN_EXTERN_C           extern "C" {
#define	PR_END_EXTERN_C             }

#define ENTER_CRITICAL_SECTION( )   __disable_irq()
#define EXIT_CRITICAL_SECTION( )    __enable_irq()
#define assert( expr )

void MODBUS_UART_IRQHandler(UART_HandleTypeDef *huart);

typedef char            BOOL;
typedef unsigned char  UCHAR;
typedef char            CHAR;
typedef unsigned short USHORT;
typedef short           SHORT;
typedef unsigned long  ULONG;
typedef long            LONG;

#ifndef TRUE
  #define TRUE  1
#endif

#ifndef FALSE
  #define FALSE 0
#endif

#endif
