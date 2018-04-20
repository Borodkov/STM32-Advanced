/**
  ******************************************************************************
    @file
    @author
    @version
    @date
    @brief   LCD Log configuration file.
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef  __LCD_LOG_CONF_H
#define  __LCD_LOG_CONF_H

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "stm32746g_discovery_lcd.h"

/* Comment the line below to disable the scroll back and forward features */
#define     LCD_SCROLL_ENABLED      0

/* Define the Fonts  */
#define     LCD_LOG_HEADER_FONT                   Font16
#define     LCD_LOG_FOOTER_FONT                   Font12
#define     LCD_LOG_TEXT_FONT                     Font12

/* Define the LCD LOG Color  */
#define     LCD_LOG_BACKGROUND_COLOR              LCD_COLOR_BLACK
#define     LCD_LOG_TEXT_COLOR                    LCD_COLOR_WHITE
#define     LCD_LOG_DEFAULT_COLOR                 LCD_COLOR_WHITE

#define     LCD_LOG_SOLID_BACKGROUND_COLOR        LCD_COLOR_DARKGRAY
#define     LCD_LOG_SOLID_TEXT_COLOR              LCD_COLOR_WHITE

/* Define the cache depth */
#define     CACHE_SIZE               17
#define     YWINDOW_SIZE             17

#if (YWINDOW_SIZE > 17)
#error "Wrong YWINDOW SIZE"
#endif

/* Redirect the printf to the LCD */
#ifdef __GNUC__
/*  With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
    set to 'Yes') calls __io_putchar() */
#define LCD_LOG_PUTCHAR int __io_putchar(int ch)
#else
#define LCD_LOG_PUTCHAR int fputc(int ch, FILE *f)
#endif /* __GNUC__ */

#endif /* __LCD_LOG_CONF_H */
