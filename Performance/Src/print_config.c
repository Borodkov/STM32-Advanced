/**
 ******************************************************************************
    @file    stm32f7_performances/Src/print_config.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This file contains the routine that prints the different
            system configurations.
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "print_config.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
const char *pProjecNamesList[9] = {
    "1 - Exec in Flash ITCM - Data in DTCM",
    "2 - Exec in Flash ITCM - Data in SRAM1",
    "3 - Exec in Flash AXI - Data in DTCM",
    "4 - Exec in Flash AXI - Data in SRAM1",
    "5 - Exec in Ram ITCM - Data in DTCM",
#if ((PROJ_MENU_NB == 6) && (PROJ_SUB_MENU_NB==1))
    "6_1 - Exec in QuadSPI Flash - Data in DTCM\n\r (const in QSPI Flash)",
#else
    "6_2 - Exec in QuadSPI Flash - Data in DTCM\n\r (const in Flash TCM)",
#endif
    "7 - Exec in ExtSRAM swapped - Data in DTCM\n\r (const in SRAM)"
};

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/**
    @brief  Print the current system configuration.
    @param  None
    @retval None
*/
void Print_config(void) {
    uint32_t flash_acr = 0;
    uint32_t scb_ccr = 0;
#if ((PROJ_MENU_NB == 1) || (PROJ_MENU_NB == 2) || (PROJ_MENU_NB == 3) || \
      (PROJ_MENU_NB == 4) || ((PROJ_MENU_NB == 6) && (PROJ_SUB_MENU_NB == 2)))
    uint32_t flash_wide = 0;
#endif
#if defined (DATA_IN_QSPI)
    uint32_t quadspi_cr = 0;
    uint32_t quadspi_ccr = 0;
#endif
#if defined (DATA_IN_ExtSDRAM)
    uint32_t fmc_bank56_sdr0 = 0;
#endif
    printf("-------------------------------------------------------\n\r");
    printf(" Config.: %s\n\r", pProjecNamesList[PROJ_MENU_NB - 1]);
    printf("-------------------------------------------------------\n\r");
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
    printf("-------------------------------------------------------\n\r");
    printf("| Flash WS | Width | ART | ART-PF | D-cache | I-cache |\n\r");
    printf("-----------+-------+-----+--------+---------+----------\n\r");
    flash_acr = FLASH->ACR;
    scb_ccr = SCB->CCR;
#if ((PROJ_MENU_NB == 5) || ((PROJ_MENU_NB == 6) && (PROJ_SUB_MENU_NB==1)) \
    || (PROJ_MENU_NB == 7))
    printf("|    NA    |  NA   | %s |   %s  |   %s   |   %s   |\n\r", \
           ((flash_acr & 0x200) ? ON : OFF), \
           ((flash_acr & 0x100) ? ON : OFF), \
           ((scb_ccr & 0x10000) ? ON : OFF), \
           ((scb_ccr & 0x20000) ? ON : OFF));
#else

    if (HAL_GetDEVID() == 0x449) { /* STM32F74xxx/STM32F75xxx */
        flash_wide = 256;
    } else if (HAL_GetDEVID() == 0x451) { /* STM32F76xxx/STM32F77xxx */
        flash_wide = (uint32_t)((*(uint32_t *)(0x1FFF0008) & (uint32_t)(1 << 13)) ? 256 : 128);
    } else if (HAL_GetDEVID() == 0x452) { /* STM32F72xxx/STM32F73xxx */
        flash_wide = 128;
    }

    printf("|    %2lu    |  %lu  | %s |  %s   |   %s   |   %s   |\n\r", \
           flash_acr & 0xF, \
           flash_wide, \
           ((flash_acr & 0x200) ? ON : OFF), \
           ((flash_acr & 0x100) ? ON : OFF), \
           ((scb_ccr & 0x10000) ? ON : OFF), \
           ((scb_ccr & 0x20000) ? ON : OFF));
#endif
    printf("-------------------------------------------------------\n\r");
#if defined (DATA_IN_QSPI) /* QSPI access */
    quadspi_cr = QUADSPI->CR;
    quadspi_ccr = QUADSPI->CCR;
    printf(" QuadSPI config:\n\r");
    printf("----------------------------------------------------\n\r");
    printf("| Prescaler | QSPI CLK | DDRMODE |  Mode  |XIP/SIOO|\n\r");
    printf("------------+----------+---------+--------+---------\n\r");
    printf("|   %3lu     |  %3luMHz  |   %3s   | %1s-4-4  |   %s  |\n\r", \
           ((quadspi_cr & 0xFF000000) >> 24), \
           (HAL_RCC_GetSysClockFreq() / 1000000) / (((quadspi_cr & 0xFF000000) >> 24) + 1), \
           ((quadspi_ccr & 0x80000000) ? ON : OFF), \
           ((quadspi_ccr & 0x00000300) == 0x00000300 ? "4" : "1"), \
           /* Checking on XIP mode = checking on SIOO bit value */
           ((quadspi_ccr & 0x10000000) != 0 ? ON : OFF)
          );
    printf("----------------------------------------------------\n\r");
#endif      /* DATA_IN_QSPI */
#if defined (DATA_IN_ExtSDRAM) /* SDRAM access */
    fmc_bank56_sdr0 = FMC_Bank5_6->SDCR[0];
    printf(" FMC SDRAM config:\n\r");
    printf("---------------------------------\n\r");
    printf("| Mem CLK freq  | Mem Bus width |\n\r");
    printf("----------------+----------------\n\r");
    printf("|   %3.1f MHz   |      %2d       |\n\r", \
           ((fmc_bank56_sdr0 & 0xC00) >> 10) == 0 ? \
           (float)0.0 : ((float)(HAL_RCC_GetSysClockFreq() / 1000000) / ((fmc_bank56_sdr0 & 0xC00) >> 10)), \
           8 << ((fmc_bank56_sdr0 & 0x30) >> 4));
    printf("---------------------------------\n\r");
#endif  /* DATA_IN_ExtSDRAM */
}
