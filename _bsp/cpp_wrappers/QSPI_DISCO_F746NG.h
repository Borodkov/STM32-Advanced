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

#ifndef __QSPI_DISCO_F746NG_H
#define __QSPI_DISCO_F746NG_H

#include "stm32746g_discovery_qspi.h"

/*
    Class to drive the QSPI external memory (N25Q128A device)
    present on DISCO_F746NG board.

    Usage:

    #include "mbed.h"
    #include "QSPI_DISCO_F746NG.h"

    QSPI_DISCO_F746NG qspi;

    #define BUFFER_SIZE         ((uint32_t)32)
    #define WRITE_READ_ADDR     ((uint32_t)0x0050)

    int main()
    {
    uint8_t WriteBuffer[BUFFER_SIZE] = "Hello World";
    uint8_t ReadBuffer[BUFFER_SIZE];

    qspi.Init();

    // Erase memory
    qspi.Erase_Block(WRITE_READ_ADDR);

    // Write memory
    qspi.Write(WriteBuffer, WRITE_READ_ADDR, 11);

    // Read memory
    qspi.Read(ReadBuffer, WRITE_READ_ADDR, 11);
    ReadBuffer[11] = '\0';
    printf("Buffer read [%s]\n", ReadBuffer);
    }

*/
class QSPI_DISCO_F746NG {

  public:
    //! Constructor
    QSPI_DISCO_F746NG();

    //! Destructor
    ~QSPI_DISCO_F746NG();

    /**
        @brief  Initializes the QSPI interface.
        @retval QSPI memory status
    */
    uint8_t Init(void);

    /**
        @brief  De-Initializes the QSPI interface.
        @retval QSPI memory status
    */
    uint8_t DeInit(void);

    /**
        @brief  Reads an amount of data from the QSPI memory.
        @param  pData: Pointer to data to be read
        @param  ReadAddr: Read start address
        @param  Size: Size of data to read
        @retval QSPI memory status
    */
    uint8_t Read(uint8_t *pData, uint32_t ReadAddr, uint32_t Size);

    /**
        @brief  Writes an amount of data to the QSPI memory.
        @param  pData: Pointer to data to be written
        @param  WriteAddr: Write start address
        @param  Size: Size of data to write
        @retval QSPI memory status
    */
    uint8_t Write(uint8_t *pData, uint32_t WriteAddr, uint32_t Size);

    /**
        @brief  Erases the specified block of the QSPI memory.
        @param  BlockAddress: Block address to erase
        @retval QSPI memory status
    */
    uint8_t Erase_Block(uint32_t BlockAddress);

    /**
        @brief  Erases the entire QSPI memory.
        @retval QSPI memory status
    */
    uint8_t Erase_Chip(void);

    /**
        @brief  Reads current status of the QSPI memory.
        @retval QSPI memory status
    */
    uint8_t GetStatus(void);

    /**
        @brief  Return the configuration of the QSPI memory.
        @param  pInfo: pointer on the configuration structure
        @retval QSPI memory status
    */
    uint8_t GetInfo(QSPI_Info *pInfo);

    /**
        @brief  Configure the QSPI in memory-mapped mode
        @retval QSPI memory status
    */
    uint8_t EnableMemoryMappedMode(void);

  private:

};

#endif
