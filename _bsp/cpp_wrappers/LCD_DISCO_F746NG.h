/*  Copyright (c) 2010-2011 mbed.org, MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef __LCD_DISCO_F746NG_H
#define __LCD_DISCO_F746NG_H

#include "stm32746g_discovery_lcd.h"

/*
    This class drives the LCD display (RK043FN48H-CT672B 4,3" 480x272 pixels device) present on DISCO_F746NG board.

    Usage:

    #include "mbed.h"
    #include "LCD_DISCO_F746NG.h"

    LCD_DISCO_F746NG lcd;

    int main()
    {
      lcd.DisplayStringAt(0, LINE(1), (uint8_t *)"MBED EXAMPLE", CENTER_MODE);
      wait(1);
      lcd.Clear(LCD_COLOR_BLUE);
      lcd.SetBackColor(LCD_COLOR_BLUE);
      lcd.SetTextColor(LCD_COLOR_WHITE);
      lcd.DisplayStringAt(0, LINE(5), (uint8_t *)"DISCOVERY STM32F746NG", CENTER_MODE);
      while(1)
      {
      }
    }
*/
class LCD_DISCO_F746NG {

  public:
    //! Constructor
    LCD_DISCO_F746NG();

    //! Destructor
    ~LCD_DISCO_F746NG();

    /**
        @brief  Initializes the LCD.
        @retval LCD state
    */
    uint8_t Init(void);

    /**
        @brief  DeInitializes the LCD.
        @retval LCD state
    */
    uint8_t DeInit(void);

    /**
        @brief  Gets the LCD X size.
        @retval Used LCD X size
    */
    uint32_t GetXSize(void);

    /**
        @brief  Gets the LCD Y size.
        @retval Used LCD Y size
    */
    uint32_t GetYSize(void);

    /**
        @brief  Set the LCD X size.
        @param  imageWidthPixels : image width in pixels unit
        @retval None
    */
    void SetXSize(uint32_t imageWidthPixels);

    /**
        @brief  Set the LCD Y size.
        @param  imageHeightPixels : image height in lines unit
        @retval None
    */
    void SetYSize(uint32_t imageHeightPixels);

    /**
        @brief  Initializes the LCD layer in ARGB8888 format (32 bits per pixel);.
        @param  LayerIndex: Layer foreground or background
        @param  FB_Address: Layer frame buffer
        @retval None
    */
    void LayerDefaultInit(uint16_t LayerIndex, uint32_t FB_Address);

    /**
        @brief  Initializes the LCD layer in RGB565 format (16 bits per pixel);.
        @param  LayerIndex: Layer foreground or background
        @param  FB_Address: Layer frame buffer
        @retval None
    */
    void LayerRgb565Init(uint16_t LayerIndex, uint32_t FB_Address);

    /**
        @brief  Selects the LCD Layer.
        @param  LayerIndex: Layer foreground or background
        @retval None
    */
    void SelectLayer(uint32_t LayerIndex);

    /**
        @brief  Sets an LCD Layer visible
        @param  LayerIndex: Visible Layer
        @param  State: New state of the specified layer
                 This parameter can be one of the following values:
                   @arg  ENABLE
                   @arg  DISABLE
        @retval None
    */
    void SetLayerVisible(uint32_t LayerIndex, FunctionalState State);

    /**
        @brief  Configures the transparency.
        @param  LayerIndex: Layer foreground or background.
        @param  Transparency: Transparency
                  This parameter must be a number between Min_Data = 0x00 and Max_Data = 0xFF
        @retval None
    */
    void SetTransparency(uint32_t LayerIndex, uint8_t Transparency);

    /**
        @brief  Sets an LCD layer frame buffer address.
        @param  LayerIndex: Layer foreground or background
        @param  Address: New LCD frame buffer value
        @retval None
    */
    void SetLayerAddress(uint32_t LayerIndex, uint32_t Address);

    /**
        @brief  Sets display window.
        @param  LayerIndex: Layer index
        @param  Xpos: LCD X position
        @param  Ypos: LCD Y position
        @param  Width: LCD window width
        @param  Height: LCD window height
        @retval None
    */
    void SetLayerWindow(uint16_t LayerIndex, uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);

    /**
        @brief  Configures and sets the color keying.
        @param  LayerIndex: Layer foreground or background
        @param  RGBValue: Color reference
        @retval None
    */
    void SetColorKeying(uint32_t LayerIndex, uint32_t RGBValue);

    /**
        @brief  Disables the color keying.
        @param  LayerIndex: Layer foreground or background
        @retval None
    */
    void ResetColorKeying(uint32_t LayerIndex);

    /**
        @brief  Sets the LCD text color.
        @param  Color: Text color code ARGB(8-8-8-8);
        @retval None
    */
    void SetTextColor(uint32_t Color);

    /**
        @brief  Gets the LCD text color.
        @retval Used text color.
    */
    uint32_t GetTextColor(void);

    /**
        @brief  Sets the LCD background color.
        @param  Color: Layer background color code ARGB(8-8-8-8);
        @retval None
    */
    void SetBackColor(uint32_t Color);

    /**
        @brief  Gets the LCD background color.
        @retval Used background colour
    */
    uint32_t GetBackColor(void);

    /**
        @brief  Sets the LCD text font.
        @param  fonts: Layer font to be used
        @retval None
    */
    void SetFont(sFONT *fonts);

    /**
        @brief  Gets the LCD text font.
        @retval Used layer font
    */
    sFONT *GetFont(void);

    /**
        @brief  Reads an LCD pixel.
        @param  Xpos: X position
        @param  Ypos: Y position
        @retval RGB pixel color
    */
    uint32_t ReadPixel(uint16_t Xpos, uint16_t Ypos);

    /**
        @brief  Clears the whole LCD.
        @param  Color: Color of the background
        @retval None
    */
    void Clear(uint32_t Color);

    /**
        @brief  Clears the selected line.
        @param  Line: Line to be cleared
        @retval None
    */
    void ClearStringLine(uint32_t Line);

    /**
        @brief  Displays one character.
        @param  Xpos: Start column address
        @param  Ypos: Line where to display the character shape.
        @param  Ascii: Character ascii code
                  This parameter must be a number between Min_Data = 0x20 and Max_Data = 0x7E
        @retval None
    */
    void DisplayChar(uint16_t Xpos, uint16_t Ypos, uint8_t Ascii);

    /**
        @brief  Displays characters on the LCD.
        @param  Xpos: X position (in pixel);
        @param  Ypos: Y position (in pixel);
        @param  Text: Pointer to string to display on LCD
        @param  Mode: Display mode
                 This parameter can be one of the following values:
                   @arg  CENTER_MODE
                   @arg  RIGHT_MODE
                   @arg  LEFT_MODE
        @retval None
    */
    void DisplayStringAt(uint16_t Xpos, uint16_t Ypos, uint8_t *Text, Text_AlignModeTypdef Mode);

    /**
        @brief  Displays a maximum of 60 characters on the LCD.
        @param  Line: Line where to display the character shape
        @param  ptr: Pointer to string to display on LCD
        @retval None
    */
    void DisplayStringAtLine(uint16_t Line, uint8_t *ptr);

    /**
        @brief  Draws an horizontal line.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  Length: Line length
        @retval None
    */
    void DrawHLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);

    /**
        @brief  Draws a vertical line.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  Length: Line length
        @retval None
    */
    void DrawVLine(uint16_t Xpos, uint16_t Ypos, uint16_t Length);

    /**
        @brief  Draws an uni-line (between two points);.
        @param  x1: Point 1 X position
        @param  y1: Point 1 Y position
        @param  x2: Point 2 X position
        @param  y2: Point 2 Y position
        @retval None
    */
    void DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);

    /**
        @brief  Draws a rectangle.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  Width: Rectangle width
        @param  Height: Rectangle height
        @retval None
    */
    void DrawRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);

    /**
        @brief  Draws a circle.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  Radius: Circle radius
        @retval None
    */
    void DrawCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);

    /**
        @brief  Draws an poly-line (between many points);.
        @param  Points: Pointer to the points array
        @param  PointCount: Number of points
        @retval None
    */
    void DrawPolygon(pPoint Points, uint16_t PointCount);

    /**
        @brief  Draws an ellipse on LCD.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  XRadius: Ellipse X radius
        @param  YRadius: Ellipse Y radius
        @retval None
    */
    void DrawEllipse(int Xpos, int Ypos, int XRadius, int YRadius);

    /**
        @brief  Draws a pixel on LCD.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  RGB_Code: Pixel color in ARGB mode (8-8-8-8);
        @retval None
    */
    void DrawPixel(uint16_t Xpos, uint16_t Ypos, uint32_t RGB_Code);

    /**
        @brief  Draws a bitmap picture loaded in the internal Flash in ARGB888 format (32 bits per pixel);.
        @param  Xpos: Bmp X position in the LCD
        @param  Ypos: Bmp Y position in the LCD
        @param  pbmp: Pointer to Bmp picture address in the internal Flash
        @retval None
    */
    void DrawBitmap(uint32_t Xpos, uint32_t Ypos, uint8_t *pbmp);

    /**
        @brief  Draws a full rectangle.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  Width: Rectangle width
        @param  Height: Rectangle height
        @retval None
    */
    void FillRect(uint16_t Xpos, uint16_t Ypos, uint16_t Width, uint16_t Height);

    /**
        @brief  Draws a full circle.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  Radius: Circle radius
        @retval None
    */
    void FillCircle(uint16_t Xpos, uint16_t Ypos, uint16_t Radius);

    /**
        @brief  Draws a full poly-line (between many points);.
        @param  Points: Pointer to the points array
        @param  PointCount: Number of points
        @retval None
    */
    void FillPolygon(pPoint Points, uint16_t PointCount);

    /**
        @brief  Draws a full ellipse.
        @param  Xpos: X position
        @param  Ypos: Y position
        @param  XRadius: Ellipse X radius
        @param  YRadius: Ellipse Y radius
        @retval None
    */
    void FillEllipse(int Xpos, int Ypos, int XRadius, int YRadius);

    /**
        @brief  Enables the display.
        @retval None
    */
    void DisplayOn(void);

    /**
        @brief  Disables the display.
        @retval None
    */
    void DisplayOff(void);

  private:

};

#endif
