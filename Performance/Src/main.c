/**
  ******************************************************************************
    @file    stm32f7_performances/Src/main.c
    @author  MCD Application Team
    @version V2.0.0
    @date    24-February-2017
    @brief   This example shows the performances of the CPU access to different
             memories of STM32F74x and STM32F75x devices.
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "arm_math.h"
#include "arm_const_structs.h"
#include "main.h"
#include <stdlib.h>
#include "sysinit.h"
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
#define TimerCount_DeInit()  do{\
                            SysTick->LOAD = (uint32_t)0;    /* set reload register */\
                            SysTick->VAL = (uint32_t)0;  /* Clear Counter */\
                            SysTick->CTRL = (uint32_t)0x0;  /* Enable Counting*/\
                            }while(0)

#define TimerCount_Start()  do{\
                            SysTick->LOAD = (uint32_t)0xFFFFFF;  /* set reload register */\
                            SysTick->VAL = (uint32_t)0;  /* Clear Counter */\
                            SysTick->CTRL = (uint32_t)0x7;  /* Enable Counting*/\
                            }while(0)

/* Systick Stop and retrieve CPU Clocks count */
#define TimerCount_Stop(value)  do {\
                          value = SysTick->VAL;  /* Load the SysTick Counter Value */\
                            SysTick->CTRL = (uint32_t)0;    /* Disable Counting */\
                            value = 0xFFFFFF - value;  /* Capture Counts in CPU Cycles*/\
                __DSB(); \
                          }while(0)

/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/**
    @brief  Main program.
    @param  None
    @retval None
*/
int main(void) {
    uint32_t max_energy_index = 0;
    float32_t a_signal_output[TEST_LENGTH_SAMPLES / 2];
    arm_status status;
    float32_t max_value;
    uint32_t cycles = 0;
    uint32_t cycles_systick_interrupt = 0;
    __IO uint32_t cycles_result = 0;
    /* Initialize the system: Clocks, Cache etc .. */
    system_init();
    /*  Count the cycle number consumed by systick handler
        (it's counted only when a systick overflow occurs) */
    TimerCount_Start();
    SysTick_Handler();
    TimerCount_Stop(cycles_systick_interrupt);
    /* Initialize the BenchmarkTick counter */
    BenchmarkTick = 0;
    TimerCount_DeInit();
    /* SysTick Initialization */
    TimerCount_Start();
    status = ARM_MATH_SUCCESS;
    /* Process the data through the CFFT/CIFFT module */
    arm_cfft_f32(&arm_cfft_sR_f32_len1024, aSignalInput_f32_10khz, \
                 I_FFT_FLAG, BIT_REVERSE_FLAG);
    /*  Process the data through the Complex Magnitude Module for
        calculating the magnitude at each bin */
    arm_cmplx_mag_f32(aSignalInput_f32_10khz, a_signal_output, FFT_SIZE);
    /* Calculates max_value and returns corresponding BIN value */
    arm_max_f32(a_signal_output, FFT_SIZE, &max_value, &max_energy_index);
    /* SysTick Stop */
    TimerCount_Stop(cycles);
    /* Compute the number of cycles consumed by the FFT algorithm */
    /* 0x1000000 -> systick is 24-bit counter */
    cycles_result = (uint64_t)(0x1000000 - cycles_systick_interrupt) * \
                    BenchmarkTick + (uint64_t)cycles;
    /* Reconfigure systick timer for HAL */
    Systick_reconfig();
    /* Init printf target  */
    Printf_Init();
    /* Print the system configuration of the current project configuration */
    Print_config();
    /*  Display the results on the given target: LCD-TFT, Hyperterminal or
        IDE-viewer */
    printf("- Systick interrupt cycles = %lu\n\r- Total number of cycles = %lu\n\r", \
           cycles_systick_interrupt, cycles_result);
    __DSB();

    if (max_energy_index != MAX_ENERGY_REF_INDEX) {
        status = ARM_MATH_TEST_FAILURE;
    }

    printf("max_value = %f status = %d\n\r", max_value, status);

    if (status != ARM_MATH_SUCCESS) {
        while (1);  /* The signal fails the PASS check */
    }

    while (1) {   /* main function does not return */
    }
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
