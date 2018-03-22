#include "main.h"
#include "SDRAM_DISCO_F746NG.h"

SDRAM_DISCO_F746NG sdram;

// DigitalOut led_green(LED1);
// DigitalOut led_red(LED2);

// Serial pc(USBTX, USBRX);

#define BUFFER_SIZE         ((uint32_t)0x0100)
#define WRITE_READ_ADDR     ((uint32_t)0x0800)

void FillBuffer(uint32_t *pBuffer, uint32_t BufferLength, uint32_t Offset);
uint8_t CompareBuffer(uint32_t *pBuffer1, uint32_t *pBuffer2, uint16_t BufferLength);

int main() {
    uint32_t WriteBuffer[BUFFER_SIZE];
    uint32_t ReadBuffer[BUFFER_SIZE];
    FMC_SDRAM_CommandTypeDef SDRAMCommandStructure;
    // pc.printf("\n\nSDRAM demo started\n");
    // led_red = 0;
    // Fill the write buffer
    FillBuffer(WriteBuffer, BUFFER_SIZE, 0xA244250F);
    // Write buffer
    sdram.WriteData(SDRAM_DEVICE_ADDR + WRITE_READ_ADDR, WriteBuffer, BUFFER_SIZE);
    // pc.printf("Write data DONE\n");
    // Issue self-refresh command to SDRAM device
    SDRAMCommandStructure.CommandMode            = FMC_SDRAM_CMD_SELFREFRESH_MODE;
    SDRAMCommandStructure.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK2;
    SDRAMCommandStructure.AutoRefreshNumber      = 1;
    SDRAMCommandStructure.ModeRegisterDefinition = 0;

    if (sdram.Sendcmd(&SDRAMCommandStructure) != HAL_OK) {
        // led_red = 1;
        // error("BSP_SDRAM_Sendcmd FAILED\n");
    }

    // SDRAM memory read back access
    SDRAMCommandStructure.CommandMode = FMC_SDRAM_CMD_NORMAL_MODE;

    if (sdram.Sendcmd(&SDRAMCommandStructure) != HAL_OK) {
        // led_red = 1;
        // error("BSP_SDRAM_Sendcmd FAILED\n");
    }

    while (1) {
        // Read back data from the SDRAM memory
        sdram.ReadData(SDRAM_DEVICE_ADDR + WRITE_READ_ADDR, ReadBuffer, BUFFER_SIZE);
        // pc.printf("\nRead data DONE\n");

        // Checking data integrity
        if (CompareBuffer(WriteBuffer, ReadBuffer, BUFFER_SIZE) != 0) {
            // led_red = !led_red;
            // pc.printf("Write/Read buffers are different\n");
        } else {
            // led_green = !led_green;
            // pc.printf("Write/Read buffers are identical\n");
        }

        // wait(1);
    }
}

/**
    @brief  Fills buffer with user predefined data.
    @param  pBuffer: pointer on the buffer to fill
    @param  BufferLength: size of the buffer to fill
    @param  Value: first value to fill on the buffer
    @retval None
*/
void FillBuffer(uint32_t *pBuffer, uint32_t BufferLength, uint32_t Value) {
    uint32_t tmpIndex = 0;

    /* Put in global buffer different values */
    for (tmpIndex = 0; tmpIndex < BufferLength; tmpIndex++) {
        pBuffer[tmpIndex] = tmpIndex + Value;
    }
}

/**
    @brief  Compares two buffers.
    @param  pBuffer1, pBuffer2: buffers to be compared.
    @param  BufferLength: buffer's length
    @retval 0: pBuffer2 identical to pBuffer1
            1: pBuffer2 differs from pBuffer1
*/
uint8_t CompareBuffer(uint32_t *pBuffer1, uint32_t *pBuffer2, uint16_t BufferLength) {
    while (BufferLength--) {
        if (*pBuffer1 != *pBuffer2) {
            return 1;
        }

        pBuffer1++;
        pBuffer2++;
    }

    return 0;
}
