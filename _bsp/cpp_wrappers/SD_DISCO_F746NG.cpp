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

#include "SD_DISCO_F746NG.h"

// Constructor
SD_DISCO_F746NG::SD_DISCO_F746NG() {
    BSP_SD_Init();
}

// Destructor
SD_DISCO_F746NG::~SD_DISCO_F746NG() {
    BSP_SD_DeInit();
}

//=================================================================================================================
// Public methods
//=================================================================================================================
uint8_t SD_DISCO_F746NG::Init(void) {
    return BSP_SD_Init();
}

uint8_t SD_DISCO_F746NG::DeInit(void) {
    return BSP_SD_DeInit();
}

uint8_t SD_DISCO_F746NG::ITConfig(void) {
    return BSP_SD_ITConfig();
}

uint8_t SD_DISCO_F746NG::IsDetected(void) {
    return BSP_SD_IsDetected();
}

uint8_t SD_DISCO_F746NG::ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout) {
    return BSP_SD_ReadBlocks(pData, ReadAddr, NumOfBlocks, Timeout);
}

uint8_t SD_DISCO_F746NG::WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout) {
    return BSP_SD_WriteBlocks(pData, WriteAddr, NumOfBlocks, Timeout);
}

uint8_t SD_DISCO_F746NG::ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks) {
    return BSP_SD_ReadBlocks_DMA(pData, ReadAddr, NumOfBlocks);
}

uint8_t SD_DISCO_F746NG::WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks) {
    return BSP_SD_WriteBlocks_DMA(pData, WriteAddr, NumOfBlocks);
}

uint8_t SD_DISCO_F746NG::Erase(uint64_t StartAddr, uint64_t EndAddr) {
    return BSP_SD_Erase(StartAddr, EndAddr);
}

uint8_t SD_DISCO_F746NG::GetCardState(void) {
    return BSP_SD_GetCardState();
}

void SD_DISCO_F746NG::GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo) {
    BSP_SD_GetCardInfo(CardInfo);
}
