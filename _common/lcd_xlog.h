/*******************************************************************************
    Description: header for the lcd_log.c file
*******************************************************************************/

#ifndef  __LCD_LOG_H__
#define  __LCD_LOG_H__

/* Includes ------------------------------------------------------------------*/
#include "lcd_log_conf.h"

#if (LCD_SCROLL_ENABLED == 1)
  #define     LCD_CACHE_DEPTH     (YWINDOW_SIZE + CACHE_SIZE)
#else
  #define     LCD_CACHE_DEPTH     YWINDOW_SIZE
#endif


typedef struct _LCD_LOG_line {
    uint8_t  line[128];
    uint32_t color;

} LCD_LOG_line;

#define  LCD_ErrLog(...)    do { \
                                 LCD_LineColor = LCD_COLOR_RED;\
                                 xprintf("ERROR: ") ;\
                                 xprintf(__VA_ARGS__);\
                                 LCD_LineColor = LCD_LOG_DEFAULT_COLOR;\
                               }while (0)

#define  LCD_UsrLog(...)    do { \
                                 LCD_LineColor = LCD_LOG_TEXT_COLOR;\
                                 xprintf(__VA_ARGS__);\
                               } while (0)


#define  LCD_DbgLog(...)    do { \
                                 LCD_LineColor = LCD_COLOR_CYAN;\
                                 xprintf(__VA_ARGS__);\
                                 LCD_LineColor = LCD_LOG_DEFAULT_COLOR;\
                               }while (0)

extern uint32_t LCD_LineColor;

void LCD_LOG_Init(void);
void LCD_LOG_DeInit(void);
void LCD_LOG_SetHeader(uint8_t *Title);
void LCD_LOG_SetFooter(uint8_t *Status);
void LCD_LOG_ClearTextZone(void);
void LCD_LOG_UpdateDisplay(void);

#if (LCD_SCROLL_ENABLED == 1)
  ErrorStatus LCD_LOG_ScrollBack(void);
  ErrorStatus LCD_LOG_ScrollForward(void);
#endif

#endif /* __LCD_LOG_H__ */
