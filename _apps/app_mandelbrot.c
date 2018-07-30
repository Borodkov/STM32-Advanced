/**
  ******************************************************************************
    @file
    @author  PavelB
    @version V1.0
    @date    03-April-2017
    @brief
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "lcd_log.h"
#include <math.h>

/* Private define ------------------------------------------------------------*/
#define  MAX_COLOR_ZONES   360 // (max 360)

#define FRACTAL_FRAME_BUFFER ((uint32_t)0xC0200000)

/* Private typedef -----------------------------------------------------------*/
typedef struct {
    uint16_t h; // 0...359 (in Grad, 0=R, 120=G, 240=B)
    uint8_t s;  // 0...100 (in %)
    uint8_t v;  // 0...100 (in %)
} COL_HSV_t;

typedef struct {
    uint8_t r;  // 0...255
    uint8_t g;  // 0...255
    uint8_t b;  // 0...255
} COL_RGB_t;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint16_t rgb_map[MAX_COLOR_ZONES];
uint16_t akt_color_zones = 0;
//Mandelbrot Settings
float width = 3.0f;
float start_real = -2.0f;
float start_imag = -1.5f;

static TS_StateTypeDef TS_State;

/* Private function prototypes -----------------------------------------------*/
void draw_mandelbrot(float x_start, float y_start, float zoom);
void set_color_map(void);
uint32_t isElement(float a, float b);

/**
    @brief
    @param
    @retval None
*/
static void task(void const *pvParameters) {
    osDelay(1000);
    uint16_t xp, yp;
    float pressed_real, pressed_imag;
    akt_color_zones = 0;
    BSP_LCD_LayerRgb565Init(LTDC_ACTIVE_LAYER, FRACTAL_FRAME_BUFFER);
    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_SetFont(&Font12);
    BSP_LCD_DisplayStringAt(10, 30, "Fractal Demo", LEFT_MODE);
    BSP_LCD_DisplayStringAt(10, 50, "tip touch to zoom in", LEFT_MODE);
    BSP_LCD_DisplayStringAt(10, 100, "press here for 360 Colors", LEFT_MODE);
    BSP_LCD_DisplayStringAt(240, 100, "press here for 180 Colors", LEFT_MODE);
    BSP_LCD_DisplayStringAt(10, 200, "press here for 90 Colors", LEFT_MODE);
    BSP_LCD_DisplayStringAt(240, 200, "press here for 45 Colors", LEFT_MODE);

    /* Infinite loop */
    for (;;) {
        osDelay(10);
        /* Get the IT status register value */
        BSP_TS_GetState(&TS_State);

        if (TS_State.touchDetected) {
            xp = TS_State.touchX[0];
            yp = TS_State.touchY[0];

            if (akt_color_zones > 0) {
                if (xp < 480 && xp >= 1 && yp < 272 && yp >= 1) {
                    pressed_real = ((width) * (float)xp) / 480.0f + start_real;
                    pressed_imag = ((width) * (float)yp) / 272.0f + start_imag;
                    width = width / 2.0f;
                    start_real = pressed_real - (width / 2.0f);
                    start_imag = pressed_imag - (width / 2.0f);
                    draw_mandelbrot(start_real, start_imag, width);
                }
            } else {
                if (xp > 0 && xp <= 240 && yp > 50 && yp <= 150) {
                    akt_color_zones = 360;
                } else if (xp > 240 && xp <= 480 && yp > 50 && yp <= 150) {
                    akt_color_zones = 180;
                } else if (xp > 0 && xp <= 240 && yp > 150 && yp <= 250) {
                    akt_color_zones = 90;
                } else if (xp > 240 && xp <= 480 && yp > 150 && yp <= 250) {
                    akt_color_zones = 45;
                }

                if (akt_color_zones > 0) {
                    BSP_LCD_Clear(LCD_COLOR_WHITE);
                    set_color_map();
                    draw_mandelbrot(start_real, start_imag, width);
                }
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
void vStartMandelbrotTask() {
    LCD_UsrLog("vStartMandelbrotTask()\n");
    /* Create that task */
    osThreadDef(Mandelbrot,
                task,
                osPriorityNormal,
                0,
                0x200);
    osThreadCreate(osThread(Mandelbrot), NULL);
}

uint32_t isElement(float a, float b) {
    float x = 0, x2, y = 0;
    uint32_t counter = 0;

    while (x * x + y * y < 2 && counter <= akt_color_zones) {
        x2 = x * x - y * y + a;
        y = 2 * x * y + b;
        x = x2;
        ++counter;
    }

    return counter;
}

void draw_mandelbrot(float real_start, float imag_start, float width) {
    uint32_t i, j, k;
    uint32_t w = BSP_LCD_GetXSize(), h = BSP_LCD_GetYSize();
    float a, b;

    for (i = 0; i < h; i++) {
        b = (float) i * width / h + imag_start;

        for (j = 0; j < w; j++) {
            a = (float) j * width / w + real_start; //-2 <= a < 1
            k = isElement(a, b);

            if (k >= akt_color_zones) {
                BSP_LCD_DrawPixel(j, i, LCD_COLOR_BLACK);
            } else {
                BSP_LCD_DrawPixel(j, i, rgb_map[k]);
            }
        }
    }
}

void RGB_2_HSV(COL_HSV_t hsv_col, COL_RGB_t *rgb_col) {
    uint8_t diff;

    // Grenzwerte
    if (hsv_col.h > 359) { hsv_col.h = 359; }

    if (hsv_col.s > 100) { hsv_col.s = 100; }

    if (hsv_col.v > 100) { hsv_col.v = 100; }

    if (hsv_col.h < 61) {
        rgb_col->r = 255;
        rgb_col->g = (425 * hsv_col.h) / 100;
        rgb_col->b = 0;
    } else if (hsv_col.h < 121) {
        rgb_col->r = 255 - (425 * (hsv_col.h - 60)) / 100;
        rgb_col->g = 255;
        rgb_col->b = 0;
    } else if (hsv_col.h < 181) {
        rgb_col->r = 0;
        rgb_col->g = 255;
        rgb_col->b = (425 * (hsv_col.h - 120)) / 100;
    } else if (hsv_col.h < 241) {
        rgb_col->r = 0;
        rgb_col->g = 255 - (425 * (hsv_col.h - 180)) / 100;
        rgb_col->b = 255;
    } else if (hsv_col.h < 301) {
        rgb_col->r = (425 * (hsv_col.h - 240)) / 100;
        rgb_col->g = 0;
        rgb_col->b = 255;
    } else {
        rgb_col->r = 255;
        rgb_col->g = 0;
        rgb_col->b = 255 - (425 * (hsv_col.h - 300)) / 100;
    }

    hsv_col.s = 100 - hsv_col.s;
    diff = ((255 - rgb_col->r) * hsv_col.s) / 100;
    rgb_col->r = rgb_col->r + diff;
    diff = ((255 - rgb_col->g) * hsv_col.s) / 100;
    rgb_col->g = rgb_col->g + diff;
    diff = ((255 - rgb_col->b) * hsv_col.s) / 100;
    rgb_col->b = rgb_col->b + diff;
    rgb_col->r = (rgb_col->r * hsv_col.v) / 100;
    rgb_col->g = (rgb_col->g * hsv_col.v) / 100;
    rgb_col->b = (rgb_col->b * hsv_col.v) / 100;
}

void set_color_map(void) {
    uint32_t n;
    uint8_t r = 0x1F, g = 0x3F, b = 0x1F;
    uint8_t step;
    COL_HSV_t hsv_col;
    COL_RGB_t rgb_col;
    hsv_col.h = 0;
    hsv_col.s = 100;
    hsv_col.v = 100;

    if (akt_color_zones > MAX_COLOR_ZONES) { akt_color_zones = MAX_COLOR_ZONES; }

    step = (360) / akt_color_zones;

    if (step < 1) { step = 1; }

    // set all colors
    for (n = 0; n < akt_color_zones; n++) {
        RGB_2_HSV(hsv_col, &rgb_col);
        r = rgb_col.r >> 3;
        g = rgb_col.g >> 2;
        b = rgb_col.b >> 3;
        rgb_map[n] = (r << 11) | (g << 5) | (b);
        hsv_col.h += step;

        if (hsv_col.h > 359) { hsv_col.h = 359; }
    }
}
