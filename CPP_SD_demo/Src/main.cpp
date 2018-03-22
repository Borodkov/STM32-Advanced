#include "main.h"
#include "SD_DISCO_F746NG.h"

SD_DISCO_F746NG sd;

// DigitalOut led_green(LED1);
// DigitalOut led_red(LED2);

// Serial pc(USBTX, USBRX);

#define BLOCK_START_ADDR         0     /* Block start address      */
#define NUM_OF_BLOCKS            5     /* Total number of blocks   */
#define BUFFER_WORDS_SIZE        ((BLOCKSIZE * NUM_OF_BLOCKS) >> 2) /* Total data size in bytes */

uint32_t aTxBuffer[BUFFER_WORDS_SIZE];
uint32_t aRxBuffer[BUFFER_WORDS_SIZE];
/* Private function prototypes -----------------------------------------------*/
void SD_main_test(void);
void SD_Detection(void);

static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLenght, uint32_t uwOffset);
static uint8_t Buffercmp(uint32_t *pBuffer1, uint32_t *pBuffer2, uint16_t BufferLength);

int main() {
    uint8_t SD_state = MSD_OK;
    // pc.printf("\n\nuSD example start:\n");
    // led_red = 0;
    SD_state = sd.Init();

    if (SD_state != MSD_OK) {
        if (SD_state == MSD_ERROR_SD_NOT_PRESENT) {
            // pc.printf("SD shall be inserted before running test\n");
        } else {
            // pc.printf("SD Initialization : FAIL.\n");
        }

        // pc.printf("SD Test Aborted.\n");
    } else {
        // pc.printf("SD Initialization : OK.\n");
        SD_state = sd.Erase(BLOCK_START_ADDR, (BLOCK_START_ADDR + NUM_OF_BLOCKS - 1));

        /* Wait until SD card is ready to use for new operation */
        while (sd.GetCardState() != SD_TRANSFER_OK) {
        }

        if (SD_state != MSD_OK) {
            // pc.printf("SD ERASE : FAILED.\n");
            // pc.printf("SD Test Aborted.\n");
        } else {
            // pc.printf("SD ERASE : OK.\n");
            /* Fill the buffer to write */
            Fill_Buffer(aTxBuffer, BUFFER_WORDS_SIZE, 0x2300);
            SD_state = sd.WriteBlocks(aTxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS, 10000);

            /* Wait until SD card is ready to use for new operation */
            while (sd.GetCardState() != SD_TRANSFER_OK) {
            }

            if (SD_state != MSD_OK) {
                // pc.printf("SD WRITE : FAILED.\n");
                // pc.printf("SD Test Aborted.\n");
            } else {
                // pc.printf("SD WRITE : OK.\n");
                SD_state = sd.ReadBlocks(aRxBuffer, BLOCK_START_ADDR, NUM_OF_BLOCKS, 10000);

                /* Wait until SD card is ready to use for new operation */
                while (sd.GetCardState() != SD_TRANSFER_OK) {
                }

                if (SD_state != MSD_OK) {
                    // pc.printf("SD READ : FAILED.\n");
                    // pc.printf("SD Test Aborted.\n");
                } else {
                    // pc.printf("SD READ : OK.\n");
                    if (Buffercmp(aTxBuffer, aRxBuffer, BUFFER_WORDS_SIZE) > 0) {
                        // pc.printf("SD COMPARE : FAILED.\n");
                        // pc.printf("SD Test Aborted.\n");
                    } else {
                        // pc.printf("SD Test : OK.\n");
                        // pc.printf("SD can be removed.\n");
                    }
                }
            }
        }
    }

    while (1) {
    }
}

/**
    @brief  Fills buffer with user predefined data.
    @param  pBuffer: pointer on the buffer to fill
    @param  uwBufferLenght: size of the buffer to fill
    @param  uwOffset: first value to fill on the buffer
    @retval None
*/
static void Fill_Buffer(uint32_t *pBuffer, uint32_t uwBufferLength, uint32_t uwOffset) {
    uint32_t tmpIndex = 0;

    /* Put in global buffer different values */
    for (tmpIndex = 0; tmpIndex < uwBufferLength; tmpIndex++) {
        pBuffer[tmpIndex] = tmpIndex + uwOffset;
    }
}

/**
    @brief  Compares two buffers.
    @param  pBuffer1, pBuffer2: buffers to be compared.
    @param  BufferLength: buffer's length
    @retval 1: pBuffer identical to pBuffer1
            0: pBuffer differs from pBuffer1
*/
static uint8_t Buffercmp(uint32_t *pBuffer1, uint32_t *pBuffer2, uint16_t BufferLength) {
    while (BufferLength--) {
        if (*pBuffer1 != *pBuffer2) {
            return 1;
        }

        pBuffer1++;
        pBuffer2++;
    }

    return 0;
}
