#include "main.h"
#include "QSPI_DISCO_F746NG.h"

QSPI_DISCO_F746NG qspi;

// DigitalOut led_green(LED1);
// DigitalOut led_red(LED2);

// Serial pc(USBTX, USBRX);

#define BUFFER_SIZE         ((uint32_t)32)
#define WRITE_READ_ADDR     ((uint32_t)0x0050)
#define QSPI_BASE_ADDR      ((uint32_t)0x90000000)

int main() {
    QSPI_Info pQSPI_Info;
    //                                  123456789012345
    uint8_t WriteBuffer[BUFFER_SIZE] = "Hello World !!!";
    uint8_t ReadBuffer[BUFFER_SIZE];
    // pc.printf("\n\nQSPI demo started\n");
    // led_red = 0;

    // Check initialization
    if (qspi.Init() != QSPI_OK) {
        // led_red = 1;
        // error("Initialization FAILED\n");
    } else {
        // pc.printf("Initialization PASSED\n");
    }

    // Check memory informations
    qspi.GetInfo(&pQSPI_Info);

    if ((pQSPI_Info.FlashSize          != N25Q128A_FLASH_SIZE) ||
            (pQSPI_Info.EraseSectorSize    != N25Q128A_SUBSECTOR_SIZE) ||
            (pQSPI_Info.ProgPageSize       != N25Q128A_PAGE_SIZE) ||
            (pQSPI_Info.EraseSectorsNumber != N25Q128A_SUBSECTOR_SIZE) ||
            (pQSPI_Info.ProgPagesNumber    != N25Q128A_SECTOR_SIZE)) {
        // led_red = 1;
        // error("Get informations FAILED\n");
    } else {
        // pc.printf("Get informations PASSED\n");
    }

    // Erase memory
    if (qspi.Erase_Block(WRITE_READ_ADDR) != QSPI_OK) {
        // led_red = 1;
        // error("Erase block FAILED\n");
    } else {
        // pc.printf("Erase block PASSED\n");
    }

    // Write memory
    if (qspi.Write(WriteBuffer, WRITE_READ_ADDR, 15) != QSPI_OK) {
        // led_red = 1;
        // error("Write FAILED\n");
    } else {
        // pc.printf("Write PASSED\n");
    }

    // Read memory
    if (qspi.Read(ReadBuffer, WRITE_READ_ADDR, 11) != QSPI_OK) {
        // led_red = 1;
        // error("Read FAILED\n");
    } else {
        ReadBuffer[11] = '\0';
        // pc.printf("Read PASSED\n");
        // pc.printf("Buffer read [%s]\n", ReadBuffer);
    }

// FIXME: This example below does not work.
#if 0

    // Read using memory-mapped mode
    if (qspi.EnableMemoryMappedMode() != QSPI_OK) {
        led_red = 1;
        error("Memory-mapped mode init FAILED\n");
    } else {
        pc.printf("Memory-mapped mode init PASSED\n");
        // Point to first address in memory
        __IO uint8_t *data_ptr = (__IO uint8_t *)(QSPI_BASE_ADDR + WRITE_READ_ADDR);

        // Read few bytes in memory
        for (uint8_t idx = 0; idx < 20; idx++) {
            pc.printf("Byte read [%c]\n", *data_ptr);
            data_ptr++; // Next memory address
        }
    }

    qspi.Init(); // Needed after memory-mapped mode is enabled
    qspi.Read(ReadBuffer, WRITE_READ_ADDR, 20);
    pc.printf("Buffer read [%s]\n", ReadBuffer);
#endif

    while (1) {
        // led_green = !led_green;
        // wait(1);
    }
}
