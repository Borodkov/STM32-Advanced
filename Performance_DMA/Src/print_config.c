/**
 ******************************************************************************
    @file    stm32f7_performances_DMAs/Src/print_config.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This file contains the routine that prints the different
            system configurations in current scenario.
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "print_config.h"
#include "main.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Different memory address ranges */
#define FLASHAXI_ADDRESS_START   0x08000000
#define FLASHAXI_ADDRESS_END     0x080FFFFF

#define FLASHITCM_ADDRESS_START  0x00200000
#define FLASHITCM_ADDRESS_END    0x002FFFFF

#define DTCMRAM_ADDRESS_START    0x20000000
#define DTCMRAM_ADDRESS_END      0x2000FFFF

#define SRAM1_ADDRESS_START      0x20010000
#define SRAM1_ADDRESS_END        0x2004BFFF

#define SRAM2_ADDRESS_START      0x2004C000
#define SRAM2_ADDRESS_END        0x2004FFFF

#define SDRAM_ADDRESS_START      0x60000000
#define SDRAM_ADDRESS_END        0x607FFFFF

/* Private macro -------------------------------------------------------------*/
#define WHICH_MEM(ADDRESS) \
((ADDRESS >= FLASHAXI_ADDRESS_START) && (ADDRESS <= FLASHAXI_ADDRESS_END)? "FLASH-AXI" : \
(ADDRESS >= FLASHITCM_ADDRESS_START) && (ADDRESS <= FLASHITCM_ADDRESS_END)? "FLASH-TCM" : \
(ADDRESS >= SRAM1_ADDRESS_START) && (ADDRESS <= SRAM1_ADDRESS_END)? "SRAM1    " : \
(ADDRESS >= SRAM2_ADDRESS_START) && (ADDRESS <= SRAM2_ADDRESS_END)? "SRAM2    " : \
(ADDRESS >= DTCMRAM_ADDRESS_START) && (ADDRESS <= DTCMRAM_ADDRESS_END)? "DTCM-RAM " : \
(ADDRESS >= SDRAM_ADDRESS_START) && (ADDRESS <= SDRAM_ADDRESS_END)? "SDRAM    " : \
 " Unknown ")

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
    @brief Print the configuration of the current scenario
    @param None
    @retval None
*/
void Print_config(void) {
    const char *p_project_list[3] = {
        "1 - Exec in Flash ITCM - Data in DTCM",
        "2 - Exec in Flash ITCM - Data in SRAM1",
        "3 - Exec in Flash AXI - Data in DTCM"
    };
    uint32_t flash_acr = 0;
    uint32_t scb_ccr = 0;
#if defined (DATA_IN_ExtSDRAM)
    uint32_t fmc_bank56_sdr0 = 0;
#endif
    /* Print the current system configuration */
    printf("----------------------------------------------------\n\r");
    printf(" Config.: %s\n\r", p_project_list[PROJ_MENU_NB - 1]);
    printf("----------------------------------------------------\n\r");
#if defined  ( __CC_ARM )
    printf(" IDE: KEIL MDK-ARM \n\r");
#elif defined  ( __ICCARM__ )
    printf(" IDE: IAR EWARM \n\r");
#elif defined ( __GNUC__ )
    printf(" IDE: AC6 SW4STM32 \n\r");
#else
    printf(" IDE: Unrecognized!! \n\r");
#endif
    printf(" Device: %s\n\r", \
           (HAL_GetDEVID() == 0x449) ? "STM32F74xxx/STM32F75xxx" : \
           (HAL_GetDEVID() == 0x451) ? "STM32F76xxx/STM32F77xxx" : \
           (HAL_GetDEVID() == 0x452) ? "STM32F72xxx/STM32F73xxx" : \
           "Unrecognized device");
    printf("----------------------------------------------------\n\r");
    printf(" System frequency: %luMHz                           \n\r", \
           HAL_RCC_GetSysClockFreq() / 1000000);
    printf("----------------------------------------------------\n\r");
    printf("| Flash WS  |   ART   | ART-PF | D-cache | I-cache |\n\r");
    printf("+-----------+---------+--------+---------+---------+\n\r");
    flash_acr = FLASH->ACR;
    scb_ccr = SCB->CCR;
    printf("|    %2lu     |   %s   |  %s   |   %s   |   %s   |\n\r", \
           flash_acr & 0xF, \
           ((flash_acr & 0x200) ? ON : OFF), \
           ((flash_acr & 0x100) ? ON : OFF), \
           ((scb_ccr & 0x10000) ? ON : OFF), \
           ((scb_ccr & 0x20000) ? ON : OFF));
    printf("----------------------------------------------------\n\r");
#if defined (DATA_IN_ExtSDRAM) /* SDRAM access */
    fmc_bank56_sdr0 = FMC_Bank5_6->SDCR[0];
    printf(" FMC SDRAM config:\n\r");
    printf("---------------------------------\n\r");
    printf("| Mem CLK freq  | Mem Bus width |\n\r");
    printf("+---------------+---------------+\n\r");
    printf("|   %3.1f MHz   |      %2d       |\n\r", ((fmc_bank56_sdr0 & 0xC00) >> 10) == 0 ? \
           (float)0.0 : ((float)(HAL_RCC_GetSysClockFreq() / 1000000) / ((fmc_bank56_sdr0 & 0xC00) >> 10)), \
           8 << ((fmc_bank56_sdr0 & 0x30) >> 4));
    printf("---------------------------------\n\r");
#endif  /* DATA_IN_ExtSDRAM */
    /* Print the current DMAs transfers scenario configuration */
    printf("                    -------------------------------------\n\r");
    printf(" DMA(s) Transfer(s) | Source    | Destin.   | Size(Byte)|\n\r");
    printf("--------------------+-----------+-----------+------------\n\r");
#if ( USE_DMA2 == 1)
    printf("| DMA2              | %s | %s |   %5d   |\n\r", \
           WHICH_MEM(DMA2_SRC_ADDRESS), \
           WHICH_MEM(DMA2_DST_ADDRESS), \
           DMA2_TRANSFER_SIZE);
#else
    printf("| DMA2              |    --     |    --     |    --     |\n\r");
#endif
    printf("--------------------+-----------+-----------+------------\n\r");
#if ( USE_DMA1_SPI == 1)
    printf("| DMA1_SPI3         | %s | %s |   %5d   |\n\r", \
           WHICH_MEM(DMA1SPI_SRC_ADDRESS), \
           WHICH_MEM(DMA1SPI_DST_ADDRESS), \
           DMA1SPI_TRANSFER_SIZE);
#else
    printf("| DMA1_SPI3         |    --     |    --     |    --     |\n\r");
#endif
    printf("---------------------------------------------------------\n\r");
}
