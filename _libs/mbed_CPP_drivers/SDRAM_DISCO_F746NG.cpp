/*  Copyright (c) 2010-2016 mbed.org, MIT License

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

#include "SDRAM_DISCO_F746NG.h"

// Constructor
SDRAM_DISCO_F746NG::SDRAM_DISCO_F746NG() {
    BSP_SDRAM_Init();
}

// Destructor
SDRAM_DISCO_F746NG::~SDRAM_DISCO_F746NG() {
    BSP_SDRAM_DeInit();
}

//=================================================================================================================
// Public methods
//=================================================================================================================
uint8_t SDRAM_DISCO_F746NG::Init(void) {
    return BSP_SDRAM_Init();
}

uint8_t SDRAM_DISCO_F746NG::DeInit(void) {
    return BSP_SDRAM_DeInit();
}

void SDRAM_DISCO_F746NG::Initialization_sequence(uint32_t RefreshCount) {
    BSP_SDRAM_Initialization_sequence(RefreshCount);
}

uint8_t SDRAM_DISCO_F746NG::ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize) {
    return BSP_SDRAM_ReadData(uwStartAddress, pData, uwDataSize);
}

uint8_t SDRAM_DISCO_F746NG::ReadData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize) {
    return BSP_SDRAM_ReadData_DMA(uwStartAddress, pData, uwDataSize);
}

uint8_t SDRAM_DISCO_F746NG::WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize) {
    return BSP_SDRAM_WriteData(uwStartAddress, pData, uwDataSize);
}

uint8_t SDRAM_DISCO_F746NG::WriteData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize) {
    return BSP_SDRAM_WriteData_DMA(uwStartAddress, pData, uwDataSize);
}

uint8_t SDRAM_DISCO_F746NG::Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd) {
    return BSP_SDRAM_Sendcmd(SdramCmd);
}
