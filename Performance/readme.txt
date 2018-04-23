/**
  @page stm32f7_performances AN4667 STM32F7 system architecture and performance Readme file
  
  @verbatim
  ******************** (C) COPYRIGHT 2017 STMicroelectronics *******************
  * @file    stm32f7_performances/readme.txt 
  * @author  MCD Application Team
  * @version V3.0.0
  * @date    24-Feb-2017
  * @brief   Description of the AN4667 "STM32F7 system architecture and
  *          performance".
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
and STM32F75xx performances in different configurations of code and data locations. 
The example demonstrates the calculation of the maximum energy in the frequency domain of the input 
signal with the use of Complex FFT, Complex Magnitude, and Maximum functions. The demo runs on STM327x6G-EVAL 
board from STMicroelectronics.
The obtained results, in term of number of cycles consumed by the FFT process, are shown on the LCD after 
running the demo.
The results can also be displayed via Hyperterminal using UART or "printf-viewer" of the toolchain.
This configuration is available only for MDK-ARM and EWARM toolchains.
* How to display the results via Hyperterminal:
- In the Project defines, please replace PRINTF_LCD by PRINTF_UART and recompile the project.
Hyperterminal config:
       Baudrate: 115200
       Data bits: 7
       Stop bits: 1
       Parity Odd
       Flow Control: None
The application uses the VCOM port of the ST-Link. So to know which COM number the board is using, 
the user should connect the ST-Link of his board to his PC through a USB cable and go to 
"Control Panel" -> "System" -> "Device Manager" -> "Ports (COM & LPT)"   

Note: the results provided in the AN are obtained using the LCD.
	   
* How to display the results via "printf-viewer" of the toolchain:
- In the Project defines, please replace PRINTF_LCD by PRINTF_VIEWER and recompile the project.	   
For MDK-ARM: 
  - load your application to your target. 
  - Start a debug session.
  - Before running the demo, go to "View" -> "Serial windows" -> "Debug (printf) Viewer"
  - Run the application.
  
For EWARM: 
  - load your application to your target. 
  - Before running the demo, go to "View" -> "Terminal I/O"
  - Run the application.  

Note: the results provided in the AN4667 are obtained using the LCD. 
	   
This example provides 8 workspaces:
 - 1-FlashITCM_rwRAM-DTCM:
code executed from Flash ITCM, constants in Flash ITCM, data storage in DTCM-RAM (ART + ART-Prefetch ON)
 
 - 2-FlashITCM_rwSRAM1:
code executed from Flash ITCM, constants in Flash ITCM, data storage in SRAM1 (ART + ART-Prefetch + D-cache ON) 

 - 3-FlashAXI_rwRAM-DTCM: 
code executed from Flash AXI, constants in Flash AXI, data storage in DTCM-RAM (I-cache ON + D-cache ON)

 - 4-FlashAXI_rwSRAM1:
code executed from Flash AXI, constants in Flash AXI, data storage in SRAM1 (I-cache ON + D-cache ON)

 - 5-RamITCM_rwRAM-DTCM:
code executed from ITCM-RAM, constants in ITCM-RAM, data storage in DTCM-RAM (nothing enabled)

 - 6_1-QuadSPI_rwRAM-DTCM:
code executed from QuadSPI Flash in DDR mode and single-quad mode, constants in QuadSPI Flash, data storage 
in DTCM-RAM (I-cache ON + D-cache ON)
 
 - 6_2-QuadSPI_rwRAM-DTCM:
code executed from QuadSPI Flash in DDR mode and single-quad mode, constants in Flash ITCM, data storage 
in DTCM-RAM (I-cache + ART + ART-Prefetch ON)

 - 7-ExtSDRAM-Swapped_rwDTCM:
code executed from FMC-SDRAM in 32 bit access at HCLK/2, constants in FMC-SDRAM, data storage 
in DTCM-RAM (I-cache ON + D-cache ON)


@par Directory contents 

  - stm32f7_performances/Src/main.c                       Contains the main of the example.
  - stm32f7_performances/Src/sysinit.c                    Contains the initialization routines of the system.
  - stm32f7_performances/Src/signal_input.c               Contains the table of the input signal. 
  - stm32f7_performances/Src/print_config.c               Contains the routine that displays the system configuration.
  - stm32f7_performances/Src/serial.c                     Contains the retarget of the putc() routine to LCD-TFT, UART and IDE viewer.           
  - stm32f7_performances/Src/system_stm32f7xx.c           STM32F7xx system clock and external memories configuration file.
  - stm32f7_performances/Src/stm32f7xx_it.c               Interrupt handlers. 
  - stm32f7_performances/Src/stm32f7xx_hal_msp.c          HAL MSP module.

  - stm32f7_performances/Inc/main.h                       Main program header file  
  - stm32f7_performances/Inc/stm32f7xx_hal_conf.h         HAL Configuration file
  - stm32f7_performances/Inc/stm32f7xx_it.h               Interrupt handlers header file
  - stm32f7_performances/Inc/print_config.h               Display configuration header file
  - stm32f7_performances/Inc/serial.h                     Retarget header file 
  - stm32f7_performances/Inc/signal_input.h               The signal input 
  - stm32f7_performances/Inc/sysinit.h                    System Initialization 
        
@par Hardware and Software environment  

  - This example runs on STM32F756xx/STM32F746xx devices.
    
  - This example has been tested with STMicroelectronics STM327x6G-EVAL revB 
    evaluation boards and can be easily tailored to any other supported device 
    and development board.

@par How to use it ? 

In order to make the program work, you must do the following :
  - Open your preferred toolchain 
  - Rebuild all files
  - Load the image to the target:
  For QSPI configurations with EWARM and SW4STM32 toolchains:
     --> Open STM32 ST-Link Utility V4.0.0 (or later), connect the EVAL board and load the corresponding hex file.
	 The hex files are located under EWARM\<configuration folder>\Exe and under SW4STM32\<configuration folder>\Debug.
  For other configurations:
     --> load your image into target memory using the toolchain.
  - Run the example


 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */