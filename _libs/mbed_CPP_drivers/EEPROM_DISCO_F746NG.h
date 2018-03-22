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

#ifndef __EEPROM_DISCO_F746NG_H
#define __EEPROM_DISCO_F746NG_H

#include "stm32746g_discovery_eeprom.h"

/*
    Class to drive a M24LR64 EEPROM.

    ===================================================================
    Note:
    The I2C EEPROM memory (M24LR64) is available on a separate ANT7-M24LR-A
    daughter board (not provided with the STM32746G_DISCOVERY board).
    This daughter board must be connected on the CN2 connector.
    ===================================================================

    Usage:

    #include "mbed.h"
    #include "EEPROM_DISCO_F746NG.h"

    EEPROM_DISCO_F746NG eep;

    #define BUFFER_SIZE         ((uint32_t)32)
    #define WRITE_READ_ADDR     ((uint32_t)0x0000)

    int main()
    {
    //                                    12345678901234567890123456789012
    uint8_t WriteBuffer[BUFFER_SIZE+1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
    uint8_t ReadBuffer[BUFFER_SIZE+1];
    uint16_t bytes_rd;

    // Check initialization
    if (eep.Init() != EEPROM_OK)
    {
      error("Initialization FAILED\n");
    }

    // Write buffer
    if (eep.WriteBuffer(WriteBuffer, WRITE_READ_ADDR, BUFFER_SIZE) != EEPROM_OK)
    {
      error("Write buffer FAILED\n");
    }

    // Read buffer
    bytes_rd = BUFFER_SIZE;
    if (eep.ReadBuffer(ReadBuffer, WRITE_READ_ADDR, &bytes_rd) != EEPROM_OK)
    {
      error("Read buffer FAILED\n");
    }
    else
    {
      ReadBuffer[BUFFER_SIZE] = '\0';
      printf("Read buffer PASSED\n");
      printf("Buffer read = [%s]\n", ReadBuffer);
      printf("Bytes read = %d\n", bytes_rd);
    }

    while(1) {
    }
    }

*/
class EEPROM_DISCO_F746NG {

  public:
    //! Constructor
    EEPROM_DISCO_F746NG();

    //! Destructor
    ~EEPROM_DISCO_F746NG();

    /**
        @brief  Initializes peripherals used by the I2C EEPROM driver.

        @note   There are 2 different versions of M24LR64 (A01 & A02);.
                    Then try to connect on 1st one (EEPROM_I2C_ADDRESS_A01);
                    and if problem, check the 2nd one (EEPROM_I2C_ADDRESS_A02);
        @retval EEPROM_OK (0); if operation is correctly performed, else return value
                different from EEPROM_OK (0);
    */
    uint32_t Init(void);

    /**
        @brief  DeInitializes the EEPROM.
        @retval EEPROM state
    */
    uint8_t DeInit(void);

    /**
        @brief  Reads a block of data from the EEPROM.
        @param  pBuffer: pointer to the buffer that receives the data read from
                the EEPROM.
        @param  ReadAddr: EEPROM's internal address to start reading from.
        @param  NumByteToRead: pointer to the variable holding number of bytes to
                be read from the EEPROM.

               @note The variable pointed by NumByteToRead is reset to 0 when all the
                     data are read from the EEPROM. Application should monitor this
                     variable in order know when the transfer is complete.

        @retval EEPROM_OK (0); if operation is correctly performed, else return value
                different from EEPROM_OK (0); or the timeout user callback.
    */
    uint32_t ReadBuffer(uint8_t *pBuffer, uint16_t ReadAddr, uint16_t *NumByteToRead);

    /**
        @brief  Writes more than one byte to the EEPROM with a single WRITE cycle.

        @note   The number of bytes (combined to write start address); must not
                cross the EEPROM page boundary. This function can only write into
                the boundaries of an EEPROM page.
                This function doesn't check on boundaries condition (in this driver
                the function WriteBuffer(); which calls BSP_EEPROM_WritePage() is
                responsible of checking on Page boundaries);.

        @param  pBuffer: pointer to the buffer containing the data to be written to
                the EEPROM.
        @param  WriteAddr: EEPROM's internal address to write to.
        @param  NumByteToWrite: pointer to the variable holding number of bytes to
                be written into the EEPROM.

               @note The variable pointed by NumByteToWrite is reset to 0 when all the
                     data are written to the EEPROM. Application should monitor this
                     variable in order know when the transfer is complete.

               @note This function just configure the communication and enable the DMA
                     channel to transfer data. Meanwhile, the user application may perform
                     other tasks in parallel.

        @retval EEPROM_OK (0); if operation is correctly performed, else return value
                different from EEPROM_OK (0); or the timeout user callback.
    */
    uint32_t WritePage(uint8_t *pBuffer, uint16_t WriteAddr, uint8_t *NumByteToWrite);

    /**
        @brief  Writes buffer of data to the I2C EEPROM.
        @param  pBuffer: pointer to the buffer  containing the data to be written
                to the EEPROM.
        @param  WriteAddr: EEPROM's internal address to write to.
        @param  NumByteToWrite: number of bytes to write to the EEPROM.
        @retval EEPROM_OK (0); if operation is correctly performed, else return value
                different from EEPROM_OK (0); or the timeout user callback.
    */
    uint32_t WriteBuffer(uint8_t *pBuffer, uint16_t WriteAddr, uint16_t NumByteToWrite);

    /**
        @brief  Wait for EEPROM Standby state.

        @note  This function allows to wait and check that EEPROM has finished the
               last operation. It is mostly used after Write operation: after receiving
               the buffer to be written, the EEPROM may need additional time to actually
               perform the write operation. During this time, it doesn't answer to
               I2C packets addressed to it. Once the write operation is complete
               the EEPROM responds to its address.

        @retval EEPROM_OK (0); if operation is correctly performed, else return value
                different from EEPROM_OK (0); or the timeout user callback.
    */
    uint32_t WaitEepromStandbyState(void);

  private:

};

#endif
