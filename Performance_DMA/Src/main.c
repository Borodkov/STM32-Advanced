/**
  ******************************************************************************
    @file    stm32f7_performances_DMAs/Src/main.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This example shows the performances of STM32F74x and STM32F75x devices
             of the CPU access to different memories when DMA(s) is (are) active.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "arm_math.h"
#include "arm_const_structs.h"
#include "main.h"
#include "end.h"
#include "system_init.h"
#include <stdlib.h>
#include "signal_input.h"
#include "stm32f7xx_it.h" /* SysTick_Handler() & BenchmarkTick */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FFT_SIZE             ((uint32_t)1024)
#define I_FFT_FLAG           ((uint8_t)0)
#define BIT_REVERSE_FLAG     ((uint32_t)1)
#define MAX_ENERGY_REF_INDEX ((uint32_t)213)

/* Private macro -------------------------------------------------------------*/
/* Systick Initialization to count CPU Clocks */
#define TimerCount_DeInit() do{\
                            SysTick->LOAD = (uint32_t)0  ;  /* set reload register */\
                            SysTick->VAL = (uint32_t)0  ;   /* Clear Counter */      \
                            SysTick->CTRL = (uint32_t)0x0 ; /* Enable Counting*/     \
                            }while(0)

#define TimerCount_Start()  do{\
                            SysTick->LOAD = 0xFFFFFF  ; /* set reload register */\
                            SysTick->VAL = (uint32_t)0  ;    /* Clear Counter */         \
                            SysTick->CTRL = (uint32_t)0x7 ; /* Enable Counting*/     \
                            }while(0)

/* Systick Stop and retrieve CPU Clocks count */
#define TimerCount_Stop(value)  do {\
                            value = SysTick->VAL;/* Load the SysTick Counter Value */\
                            SysTick->CTRL = (uint32_t)0;    /* Disable Counting */               \
                            value = 0xFFFFFF - value;/* Capture Counts in CPU Cycles*/\
                            }while(0)

/* Private variables ---------------------------------------------------------*/
info Infos;

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
    @brief  Main program.
    @param  None
    @retval None
*/
int main(void) {
    uint32_t cycles = 0;
    uint32_t cycles_systick_interrupt = 0;
#if (CHECK_TRANSFER == 1)
    /* Settled to 1 after FFT computation */
    __IO uint8_t flag_fft_algo_complete = 0;
#endif
    /* Initialize the system */
    sys_init();
    /* De-init systick configuration */
    TimerCount_DeInit();
    /* Count the cycle number consumed by systick handler */
    TimerCount_Start();
    SysTick_Handler();
    TimerCount_Stop(cycles_systick_interrupt);
    /* Save the cycles number of system tick interrupt */
    Infos.cycles_systick = cycles_systick_interrupt;
    /* Initialize the BenchmarkTick counter */
    BenchmarkTick = 0;
    /* De-init systick configuration */
    TimerCount_DeInit();
    /* SysTick Initialization */
    TimerCount_Start();
    /* Process the data through the CFFT/CIFFT module */
    arm_cfft_f32(&arm_cfft_sR_f32_len1024, aSignalInput_f32_10khz, \
                 I_FFT_FLAG, BIT_REVERSE_FLAG);
    /* SysTick Stop */
    TimerCount_Stop(cycles);
#if (CHECK_TRANSFER == 1)
    /*  Set the FFT process complete. This flag should be settled before any
        DMAx transfer compete flag */
    flag_fft_algo_complete = FLAG_END_FFT_PROCESS;
    Infos.flags |= flag_fft_algo_complete | FlagDMA2TransferComplete | FlagDMA1SPITransferComplete;
#else
    /* Compute the number of cycles consumed by the FFT algorithm */
    /* 0x1000000 -> systick is 24-bit counter */
    Infos.cycles_rslt = (uint32_t)(0x1000000 - cycles_systick_interrupt) \
                        *BenchmarkTick + (uint32_t)cycles;
#endif
    __DSB();
    /* End of the benchmark */
    end();
}

#ifdef  USE_FULL_ASSERT

/**
    @brief  Reports the name of the source file and the source line number
            where the assert_param error has occurred.
    @param  file: pointer to the source file name
    @param  line: assert_param error line source number
    @retval None
*/
void assert_failed(uint8_t *file, uint32_t line) {
    /*  User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

    /* Infinite loop */
    while (1) {
    }
}
#endif
