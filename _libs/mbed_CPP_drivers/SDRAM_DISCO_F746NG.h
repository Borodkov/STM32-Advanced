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

#include "stm32746g_discovery_sdram.h"

/*
    This class drives the SDRAM driver (MT48LC4M32B2B5-7) present on DISCO_F746NG board.

    Usage:

    #include "mbed.h"
    #include "SDRAM_DISCO_F746NG.h"

    int main()
    {
    sdram.WriteData(SDRAM_DEVICE_ADDR + WRITE_READ_ADDR, WriteBuffer, BUFFER_SIZE);
    SDRAMCommandStructure.CommandMode            = FMC_SDRAM_CMD_SELFREFRESH_MODE;
      SDRAMCommandStructure.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
      SDRAMCommandStructure.AutoRefreshNumber      = 1;
      SDRAMCommandStructure.ModeRegisterDefinition = 0;
      sdram.Sendcmd(&SDRAMCommandStructure);
      SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_NORMAL_MODE;
      sdram.Sendcmd(&SDRAMCommandStructure);
      sdram.ReadData(SDRAM_DEVICE_ADDR + WRITE_READ_ADDR, ReadBuffer, BUFFER_SIZE);
      CompareBuffer(WriteBuffer, ReadBuffer, BUFFER_SIZE);
      while(1) {
      }
    }

*/
class SDRAM_DISCO_F746NG {

  public:
    //! Constructor
    SDRAM_DISCO_F746NG();

    //! Destructor
    ~SDRAM_DISCO_F746NG();


    /**
        @brief  Initializes the SDRAM device.
        @retval SDRAM status
    */
    uint8_t Init(void);

    /**
        @brief  DeInitializes the SDRAM device.
        @retval SDRAM status
    */
    uint8_t DeInit(void);

    /**
        @brief  Programs the SDRAM device.
        @param  RefreshCount: SDRAM refresh counter value
        @retval None
    */
    void Initialization_sequence(uint32_t RefreshCount);

    /**
        @brief  Reads an amount of data from the SDRAM memory in polling mode.
        @param  uwStartAddress: Read start address
        @param  pData: Pointer to data to be read
        @param  uwDataSize: Size of read data from the memory
        @retval SDRAM status
    */
    uint8_t ReadData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);

    /**
        @brief  Reads an amount of data from the SDRAM memory in DMA mode.
        @param  uwStartAddress: Read start address
        @param  pData: Pointer to data to be read
        @param  uwDataSize: Size of read data from the memory
        @retval SDRAM status
    */
    uint8_t ReadData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);

    /**
        @brief  Writes an amount of data to the SDRAM memory in polling mode.
        @param  uwStartAddress: Write start address
        @param  pData: Pointer to data to be written
        @param  uwDataSize: Size of written data from the memory
        @retval SDRAM status
    */
    uint8_t WriteData(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);

    /**
        @brief  Writes an amount of data to the SDRAM memory in DMA mode.
        @param  uwStartAddress: Write start address
        @param  pData: Pointer to data to be written
        @param  uwDataSize: Size of written data from the memory
        @retval SDRAM status
    */
    uint8_t WriteData_DMA(uint32_t uwStartAddress, uint32_t *pData, uint32_t uwDataSize);

    /**
        @brief  Sends command to the SDRAM bank.
        @param  SdramCmd: Pointer to SDRAM command structure
        @retval SDRAM status
    */
    uint8_t Sendcmd(FMC_SDRAM_CommandTypeDef *SdramCmd);

  private:

};

#endif
