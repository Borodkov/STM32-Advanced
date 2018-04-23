/**
  @page stm32f7_performances_DMAs AN4667 STM32F7 system architecture and performance with 
   DMA activation Readme file
  
  @verbatim
  ******************** (C) COPYRIGHT 2017 STMicroelectronics *******************
  * @file    stm32f7_performances_DMAs/readme.txt 
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    24-Feb-2017
  * @brief   Description of the AN4667 "STM32F7 system architecture and
  *          performance with DMA activation".
  ******************************************************************************
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  @endverbatim

@par Example Description

This directory contains a set of source files and pre-configured projects that demonstrate the STM32F74xx 
and STM32F75xx performances where one or more DMA is activated in different configurations of code and data locations with. 
The example computes the FFT when DMA1 or/and DMA2 transfers data from memory A to memory B and therefore the 
current project configuration as well as the results in cycles number are displayed only on the LCD.

Note: the user should read carefully the section 2.3 "Project configuration of the CPU memory access with
DMA activation demonstration" in the AN4667 in order to run correctly the demo and get the results.	 

This example provides 3 workspaces:
 - 1-FlashITCM_rwRAM-DTCM:
code executed from Flash ITCM, constants in Flash ITCM, data storage in DTCM-RAM (ART + ART-Prefetch ON + D-Cache ON)
 
 - 2-FlashITCM_rwSRAM1:
code executed from Flash ITCM, constants in Flash ITCM, data storage in SRAM1 (ART + ART-Prefetch + D-cache ON) 
 NOTE: 
   - For this configuration and with scenario 2, please use all SRAM2 size: SRAM2_DMA_AVAILABLE_SIZE instead of (SRAM2_DMA_AVAILABLE_SIZE *6/7).
   - With IAR the configuration2/scenario 2 is unfortunately unavailable since the DMA2 finishes before the FFT process even with the usage of
    the full size of SRAM2.
	
 - 3-FlashAXI_rwRAM-DTCM: 
code executed from Flash AXI, constants in Flash AXI, data storage in DTCM-RAM (I-cache ON + D-cache ON)

To activate a DMAx transfer, the user should set the flags USE_DMA1_SPI and USE_DMA2 respectively to activate
respectively the transfer of DMA1 and DMA2. For more details how to use the workspace to get the performances results,
please refer to the section "Project configuration of the CPU memory access with DMA(s) activation demonstration" in the AN4667.

To use DMA1/SPI3, SPI3_MOSI (PC12) should be connected to SPI3_MISO (PC11). The microSD card should be removed from its slot.

@par Directory contents 

  - stm32f7_performances_DMAs/Src/main.c                       Contains the main of the example.
  - stm32f7_performances_DMAs/Src/signal_input.c               Contains the table of the input signal. 
  - stm32f7_performances_DMAs/Src/system_init.c                Contains the initialization part separated from the FFT process.
  - stm32f7_performances_DMAs/Src/end.c                        Contains the check of DMA transfers and display the configurations and the results.
  - stm32f7_performances_DMAs/Src/print_config.c               Contains the routine that displays the system configuration.
  - stm32f7_performances_DMAs/Src/serial.c                     Contains the retarget of the putc() routine to the LCD-TFT.           
  - stm32f7_performances_DMAs/Src/system_stm32f7xx.c           STM32F7xx system clock and external memories configuration file.
  - stm32f7_performances_DMAs/Src/stm32f7xx_it.c               Interrupt handlers. 
  - stm32f7_performances_DMAs/Src/stm32f7xx_hal_msp.c          HAL MSP module.

  - stm32f7_performances_DMAs/Inc/dma_utilities.h              DMA utilities including all memory address ranges used by DMA.
  - stm32f7_performances_DMAs/Inc/main.h                       Main program header file where the user configures DMA(s) transfers 
  - stm32f7_performances_DMAs/Inc/signal_input.h               The signal input 
  - stm32f7_performances_DMAs/system_init.h                    System init header file
  - stm32f7_performances_DMAs/end.h                            End of bechmark program header file
  - stm32f7_performances_DMAs/Inc/stm32f7xx_hal_conf.h         HAL Configuration file
  - stm32f7_performances_DMAs/Inc/stm32f7xx_it.h               Interrupt handlers header file
  - stm32f7_performances_DMAs/Inc/print_config.h               Display configuration header file
  - stm32f7_performances_DMAs/Inc/serial.h                     Retarget header file 
        
@par Hardware and Software environment  

  - This example runs on STM32F756xx/STM32F746xx devices.
    
  - This example has been tested with STMicroelectronics STM327x6G-EVAL revB 
    evaluation boards and can be easily tailored to any other supported device 
    and development board.

@par How to use it ? 

In order to make the program work, you must do the following :
  - Open your preferred toolchain 
  - Rebuild all files
  - Load your image into target memory using the toolchain.
  - Run the example

 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */