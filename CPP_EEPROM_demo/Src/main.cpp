#include "main.h"
#include "EEPROM_DISCO_F746NG.h"

EEPROM_DISCO_F746NG eep;

// DigitalOut led_green(LED1);
// DigitalOut led_red(LED2);

// Serial pc(USBTX, USBRX);

#define BUFFER_SIZE         ((uint32_t)32)
#define WRITE_READ_ADDR     ((uint32_t)0x0000)

int main() {
    //                                    12345678901234567890123456789012
    uint8_t WriteBuffer[BUFFER_SIZE + 1] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ012345";
    uint8_t ReadBuffer[BUFFER_SIZE + 1];
    uint16_t bytes_rd;
    // pc.printf("\n\nEEPROM demo started\n");
    // led_red = 0;

    // Check initialization
    if (eep.Init() != EEPROM_OK) {
        // led_red = 1;
        // error("Initialization FAILED\n");
    } else {
        // pc.printf("Initialization PASSED\n");
    }

    // Write buffer
    if (eep.WriteBuffer(WriteBuffer, WRITE_READ_ADDR, BUFFER_SIZE) != EEPROM_OK) {
        // led_red = 1;
        // error("Write buffer FAILED\n");
    } else {
        // pc.printf("Write buffer PASSED\n");
    }

    // Read buffer
    bytes_rd = BUFFER_SIZE;

    if (eep.ReadBuffer(ReadBuffer, WRITE_READ_ADDR, &bytes_rd) != EEPROM_OK) {
        // led_red = 1;
        // error("Read buffer FAILED\n");
    } else {
        ReadBuffer[BUFFER_SIZE] = '\0';
        // pc.printf("Read buffer PASSED\n");
        // pc.printf("Buffer read = [%s]\n", ReadBuffer);
        // pc.printf("Bytes read = %d\n", bytes_rd);
    }

    while (1) {
        // led_green = !led_green;
        // wait(1);
    }
}
