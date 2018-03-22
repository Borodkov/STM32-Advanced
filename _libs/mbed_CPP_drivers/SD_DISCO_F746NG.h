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

#ifndef __SDRAM_DISCO_F746NG_H
#define __SDRAM_DISCO_F746NG_H

#include "stm32746g_discovery_sd.h"

/*
    This class drives the uSD card driver mounted on DISCO_F746NG board.

    Usage:

    #include "mbed.h"
    #include "SD_DISCO_F746NG.h"

    SD_DISCO_F746NG sd;

    int main()
    {
      sd.Erase(BLOCK_START_ADDR, (BLOCKSIZE * NUM_OF_BLOCKS);
      sd.WriteBlocks(aTxBuffer, BLOCK_START_ADDR, BLOCKSIZE, NUM_OF_BLOCKS);
      sd.ReadBlocks(aRxBuffer, BLOCK_START_ADDR, BLOCKSIZE, NUM_OF_BLOCKS);
      while 1 {}

    }
*/
class SD_DISCO_F746NG {

  public:
    //! Constructor
    SD_DISCO_F746NG();

    //! Destructor
    ~SD_DISCO_F746NG();

    /**
        @brief  Initializes the SD card device.
        @retval SD status
    */
    uint8_t Init(void);

    /**
        @brief  DeInitializes the SD card device.
        @retval SD status
    */
    uint8_t DeInit(void);

    /**
        @brief  Configures Interrupt mode for SD detection pin.
        @retval Returns MSD_OK
    */
    uint8_t ITConfig(void);

    /**
        @brief  Detects if SD card is correctly plugged in the memory slot or not.
        @retval Returns if SD is detected or not
    */
    uint8_t IsDetected(void);

    /**
        @brief  Reads block(s); from a specified address in an SD card, in polling mode.
        @param  pData: Pointer to the buffer that will contain the data to transmit
        @param  ReadAddr: Address from where data is to be read
        @param  BlockSize: SD card data block size, that should be 512
        @param  NumOfBlocks: Number of SD blocks to read
        @retval SD status
    */
    uint8_t ReadBlocks(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks, uint32_t Timeout);

    /**
        @brief  Writes block(s); to a specified address in an SD card, in polling mode.
        @param  pData: Pointer to the buffer that will contain the data to transmit
        @param  WriteAddr: Address from where data is to be written
        @param  BlockSize: SD card data block size, that should be 512
        @param  NumOfBlocks: Number of SD blocks to write
        @retval SD status
    */
    uint8_t WriteBlocks(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks, uint32_t Timeout);

    /**
        @brief  Reads block(s); from a specified address in an SD card, in DMA mode.
        @param  pData: Pointer to the buffer that will contain the data to transmit
        @param  ReadAddr: Address from where data is to be read
        @param  BlockSize: SD card data block size, that should be 512
        @param  NumOfBlocks: Number of SD blocks to read
        @retval SD status
    */
    uint8_t ReadBlocks_DMA(uint32_t *pData, uint32_t ReadAddr, uint32_t NumOfBlocks);

    /**
        @brief  Writes block(s); to a specified address in an SD card, in DMA mode.
        @param  pData: Pointer to the buffer that will contain the data to transmit
        @param  WriteAddr: Address from where data is to be written
        @param  BlockSize: SD card data block size, that should be 512
        @param  NumOfBlocks: Number of SD blocks to write
        @retval SD status
    */
    uint8_t WriteBlocks_DMA(uint32_t *pData, uint32_t WriteAddr, uint32_t NumOfBlocks);

    /**
        @brief  Erases the specified memory area of the given SD card.
        @param  StartAddr: Start byte address
        @param  EndAddr: End byte address
        @retval SD status
    */
    uint8_t Erase(uint64_t StartAddr, uint64_t EndAddr);

    /**
        @brief  Gets the current SD card data status.
        @retval Data transfer state.
                 This value can be one of the following values:
                   @arg  SD_TRANSFER_OK: No data transfer is acting
                   @arg  SD_TRANSFER_BUSY: Data transfer is acting
                   @arg  SD_TRANSFER_ERROR: Data transfer error
    */
    uint8_t GetCardState(void);

    /**
        @brief  Get SD information about specific SD card.
        @param  CardInfo: Pointer to HAL_SD_CardInfoTypedef structure
        @retval None
    */
    void GetCardInfo(HAL_SD_CardInfoTypeDef *CardInfo);

  private:

};

#endif
