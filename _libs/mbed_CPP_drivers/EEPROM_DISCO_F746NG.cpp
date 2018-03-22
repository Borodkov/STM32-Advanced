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

#include "EEPROM_DISCO_F746NG.h"

// Constructor
EEPROM_DISCO_F746NG::EEPROM_DISCO_F746NG() {
    BSP_EEPROM_Init();
}

// Destructor
EEPROM_DISCO_F746NG::~EEPROM_DISCO_F746NG() {
    BSP_EEPROM_DeInit();
}

//=================================================================================================================
// Public methods
//=================================================================================================================
uint32_t EEPROM_DISCO_F746NG::Init(void) {
    return BSP_EEPROM_Init();
}

uint8_t EEPROM_DISCO_F746NG::DeInit(void) {
    return BSP_EEPROM_DeInit();
}

uint32_t EEPROM_DISCO_F746NG::ReadBuffer(uint8_t *pBuffer, uint16_t ReadAddr, uint16_t *NumByteToRead) {
    return BSP_EEPROM_ReadBuffer(pBuffer, ReadAddr, NumByteToRead);
}

uint32_t EEPROM_DISCO_F746NG::WritePage(uint8_t *pBuffer, uint16_t WriteAddr, uint8_t *NumByteToWrite) {
    return BSP_EEPROM_WritePage(pBuffer, WriteAddr, NumByteToWrite);
}

uint32_t EEPROM_DISCO_F746NG::WriteBuffer(uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite) {
    return BSP_EEPROM_WriteBuffer(pBuffer, WriteAddr, NumByteToWrite);
}

uint32_t EEPROM_DISCO_F746NG::WaitEepromStandbyState(void) {
    return BSP_EEPROM_WaitEepromStandbyState();
}
