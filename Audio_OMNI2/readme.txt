/**
  @page Audio_OMNI2 Audio 

  @verbatim
  ******************************************************************************
  * @file    Audio/Audio_OMNI2/readme.txt 
  * @author  MCD Application Team
  * @brief   Description of the Audio OmniSurround
  ******************************************************************************
  *
  * Copyright (c) 2017 STMicroelectronics International N.V. All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
   @endverbatim

@par Application Description 

This application shows how to configure and use the OMNI2 library, 
taking as use case audio playback from USB storage.

Any wave file stored under the USB Key can be opened using the FatFs 
file system and transferred to the internal SRAM using the file system. 
All the wave files properties are read from the Wave File Header.

OMNI2 lib brief description.
Detailed information about the OMNI2 libraries capabilities are available at:
  \Middlewares\ST\STM32_Audio\Addons\OMNI2\Release_Notes.html

A touch screen interface is used to:
* navigate from a file to another (Next and Previous buttons) 
* change volume by pressing vol+/vol- buttons
* pause playback and to quit playback menu by pressing Stop button. 
* change listening angle from 10 to 30
* update OMNI2 strength from 0 to 100.

Plug a headphone to hear the sound  /!\ Take care of yours ears. Default volume is 50%.

Note: The audio files provided under "/Utilities/Media/Audio" are based on a free 
music download from www.DanoSongs.com website and user can load his own audio 
(*.wav) files in the USB Key root.
If the selected input file has sample rate different then 48000Hz 
this example will exit displaying an error message.
Note that OMNI2 Audio OmniSurround effect of strength setting is not well noticeable 
with stereo headphones rather use Audio Headphones virtualization (HPV) module in this case.


@note The application needs to ensure that the SysTick time base is always set to 1 millisecond
      to have correct HAL operation.

@par Directory contents 

  - Audio_OMNI2/Src/main.c               Main program
  - Audio_OMNI2/Src/stm32f7xx_it.c       Interrupt handlers
  - Audio_OMNI2/Src/system_stm32f7xx.c   STM32F7xx system source file
  - Audio_OMNI2/Src/usbh_conf.c          Board support package for the USB host library 	
  - Audio_OMNI2/Src/waveplayer.c         Audio play file
  - Audio_OMNI2/Inc/ffconf.h             FAT file system module configuration file   
  - Audio_OMNI2/Inc/main.h               Main program header file
  - Audio_OMNI2/Inc/stm32f7xx_hal_conf.h HAL configuration file
  - Audio_OMNI2/Inc/stm32f7xx_it.h       Interrupt handlers header file
  - Audio_OMNI2/Inc/usbh_conf.h          USB driver Configuration file
  - Audio_OMNI2/Inc/waveplayer.h         Audio play header file
      
      
@par Hardware and Software environment

  - This example runs on STM32F756xx/STM32F746xx devices.
    
  - This application has been tested with STM32746G-Discovery board and can be
    easily tailored to any other supported device and development board.   

  - STM32746G-Discovery Set-up
    - Plug the USB key into the STM32746G-Discovery board through 'USB micro A-Male 
      to A-Female' cable (FS mode: connector CN13).
    - Connect a headphone to connector CN10.

@par How to use it ? 

In order to make the program work, you must do the following:
 - Open your preferred toolchain 
 - Rebuild all files and load your image into target memory
 - Run the application
 
 * <h3><center>&copy; COPYRIGHT STMicroelectronics</center></h3>
 */
