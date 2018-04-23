/**
 ******************************************************************************
    @file    stm32f7_performances_DMAs/Src/serial.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This file contains the routine that retarget the printf
            to LCD-TFT, UART or IDE viewer.
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx.h"
#include "serial.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_lcd.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#if defined   (  __GNUC__  )
/*  With GCC/RAISONANCE, small printf (option LD Linker->Libraries->Small printf
    set to 'Yes') calls __io_putchar() */
#define PUTCHAR_PROTOTYPE int __io_putchar(int c, FILE *f)
#else
#define PUTCHAR_PROTOTYPE int fputc(int c, FILE *f)
#endif /* __GNUC__ */

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
static int PutChar(int c, FILE *port);

#if defined ( PRINTF_UART )
static void Error_Handler(void);
#endif

/* Private functions ---------------------------------------------------------*/
/**
    @brief  Retargets the C library printf function to the USART.
    @param  None
    @retval None
*/
PUTCHAR_PROTOTYPE {
    return (PutChar(c, f));
}

/**
    @brief  Initialize the retarget of printf
    @param  None
    @retval None
*/
void Printf_Init(void) {
    /* Initialize the LCD */
    BSP_LCD_Init();
    /* Initialise the LCD Layers */
    BSP_LCD_LayerDefaultInit(1, LCD_FB_START_ADDRESS);
    /* Set LCD Foreground Layer  */
    BSP_LCD_SelectLayer(1);
    BSP_LCD_SetFont(&Font16);
    /* Clear the LCD and set the back color */
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_Clear(LCD_COLOR_BLACK);
    /* Set the LCD Text Color */
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
}

/**
    @brief  Print a character to a given target.
    @param  c: the character to print.
    @param  port: the file to write to it.
    @retval the character to print.
*/
static int PutChar(int c, FILE *port) {
    static __IO uint32_t lcd_line = 0;
    static __IO uint32_t lcd_char_pos = 0;
    sFONT *lcd_font;
    uint16_t height = 0;
    lcd_font = BSP_LCD_GetFont();

    if (c == '\n') { /* Go to new line */
        lcd_line++;
    } else if (c == '\r') { /* Carriage return */
        lcd_char_pos = 0;
    } else {
        height = lcd_line * lcd_font->Height;
        BSP_LCD_DisplayChar(lcd_char_pos * lcd_font->Width, height, c);
        lcd_char_pos++;
    }

    if (BSP_LCD_GetXSize() < ((lcd_char_pos + 1) * lcd_font->Width)) {
        lcd_line++;
        lcd_char_pos = 0;
    }

    return (c);
}
