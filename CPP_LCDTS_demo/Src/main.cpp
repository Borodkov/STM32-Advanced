#include "main.h"
#include "TS_DISCO_F746NG.h"
#include "LCD_DISCO_F746NG.h"

LCD_DISCO_F746NG lcd;
TS_DISCO_F746NG ts;

int main() {
    TS_StateTypeDef TS_State;
    uint16_t x, y;
    uint8_t text[30];
    uint8_t status;
    uint8_t idx;
    uint8_t cleared = 0;
    uint8_t prev_nb_touches = 0;
    lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN DEMO", CENTER_MODE);
    // wait(1);
    status = ts.Init(lcd.GetXSize(), lcd.GetYSize());

    if (status != TS_OK) {
        lcd.Clear(LCD_COLOR_RED);
        lcd.SetBackColor(LCD_COLOR_RED);
        lcd.SetTextColor(LCD_COLOR_WHITE);
        lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT FAIL", CENTER_MODE);
    } else {
        lcd.Clear(LCD_COLOR_GREEN);
        lcd.SetBackColor(LCD_COLOR_GREEN);
        lcd.SetTextColor(LCD_COLOR_WHITE);
        lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"TOUCHSCREEN INIT OK", CENTER_MODE);
    }

    // wait(1);
    lcd.SetFont(&Font12);
    lcd.SetBackColor(LCD_COLOR_BLUE);
    lcd.SetTextColor(LCD_COLOR_WHITE);

    while (1) {
        ts.GetState(&TS_State);

        if (TS_State.touchDetected) {
            // Clear lines corresponding to old touches coordinates
            if (TS_State.touchDetected < prev_nb_touches) {
                for (idx = (TS_State.touchDetected + 1); idx <= 5; idx++) {
                    lcd.ClearStringLine(idx);
                }
            }

            prev_nb_touches = TS_State.touchDetected;
            cleared = 0;
            sprintf((char *)text, "Touches: %d", TS_State.touchDetected);
            lcd.DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);

            for (idx = 0; idx < TS_State.touchDetected; idx++) {
                x = TS_State.touchX[idx];
                y = TS_State.touchY[idx];
                sprintf((char *)text, "Touch %d: x=%d y=%d    ", idx + 1, x, y);
                lcd.DisplayStringAt(0, LINE(idx + 1), (uint8_t *)&text, LEFT_MODE);
            }

            lcd.DrawPixel(TS_State.touchX[0], TS_State.touchY[0], LCD_COLOR_ORANGE);
        } else {
            if (!cleared) {
                lcd.Clear(LCD_COLOR_BLUE);
                sprintf((char *)text, "Touches: 0");
                lcd.DisplayStringAt(0, LINE(0), (uint8_t *)&text, LEFT_MODE);
                cleared = 1;
            }
        }
    }
}
