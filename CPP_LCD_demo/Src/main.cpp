#include "main.h"
#include "LCD_DISCO_F746NG.h"

LCD_DISCO_F746NG lcd;

// DigitalOut led1(LED1);

int main() {
    // led1 = 1;
    lcd.DisplayStringAt(0, LINE(1), (uint8_t *)"MBED EXAMPLE", CENTER_MODE);
    // wait(1);

    while (1) {
        lcd.Clear(LCD_COLOR_BLUE);
        lcd.SetBackColor(LCD_COLOR_BLUE);
        lcd.SetTextColor(LCD_COLOR_WHITE);
        // wait(0.3);
        lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"DISCOVERY STM32F746NG", CENTER_MODE);
        // wait(1);
        lcd.Clear(LCD_COLOR_GREEN);
        lcd.SetTextColor(LCD_COLOR_BLUE);
        lcd.DrawRect(10, 20, 50, 50);
        // wait(0.1);
        lcd.SetTextColor(LCD_COLOR_BROWN);
        lcd.DrawCircle(80, 80, 50);
        // wait(0.1);
        lcd.SetTextColor(LCD_COLOR_YELLOW);
        lcd.DrawEllipse(150, 150, 50, 100);
        // wait(0.1);
        lcd.SetTextColor(LCD_COLOR_RED);
        lcd.FillCircle(200, 200, 40);
        // wait(1);
        lcd.SetBackColor(LCD_COLOR_ORANGE);
        lcd.SetTextColor(LCD_COLOR_CYAN);
        lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"HAVE FUN !!!", CENTER_MODE);
        // wait(1);
        // led1 = !led1;
    }
}
