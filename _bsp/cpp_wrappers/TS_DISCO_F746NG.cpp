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

#include "TS_DISCO_F746NG.h"

// Constructor
TS_DISCO_F746NG::TS_DISCO_F746NG() {
    BSP_TS_Init(100, 100);
}

// Destructor
TS_DISCO_F746NG::~TS_DISCO_F746NG() {
    BSP_TS_DeInit();
}

//=================================================================================================================
// Public methods
//=================================================================================================================
uint8_t TS_DISCO_F746NG::Init(uint16_t ts_SizeX, uint16_t ts_SizeY) {
    return BSP_TS_Init(ts_SizeX, ts_SizeY);
}

uint8_t TS_DISCO_F746NG::DeInit(void) {
    return BSP_TS_DeInit();
}

uint8_t TS_DISCO_F746NG::ITConfig(void) {
    return BSP_TS_ITConfig();
}

uint8_t TS_DISCO_F746NG::ITGetStatus(void) {
    return BSP_TS_ITGetStatus();
}

uint8_t TS_DISCO_F746NG::GetState(TS_StateTypeDef *TS_State) {
    return BSP_TS_GetState(TS_State);
}

uint8_t TS_DISCO_F746NG::Get_GestureId(TS_StateTypeDef *TS_State) {
    return BSP_TS_Get_GestureId(TS_State);
}

void TS_DISCO_F746NG::ITClear(void) {
    BSP_TS_ITClear();
}

uint8_t TS_DISCO_F746NG::ResetTouchData(TS_StateTypeDef *TS_State) {
    return BSP_TS_ResetTouchData(TS_State);
}
