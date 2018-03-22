/*  Copyright (c) 2010-2016 mbed.org, MIT License

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software
    and associated documentation files (the "Software"), to deal in the Software without
    restriction, including without limitation the rights to use, copy, modify, merge, publish,
    distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or
    substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
    BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
    DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "AUDIO_DISCO_F746NG.h"

// Constructor
AUDIO_DISCO_F746NG::AUDIO_DISCO_F746NG() {
    BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_DIGITAL_MICROPHONE_2, OUTPUT_DEVICE_HEADPHONE, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
}

// Destructor
AUDIO_DISCO_F746NG::~AUDIO_DISCO_F746NG() {
}

//=================================================================================================================
// Public methods
//=================================================================================================================
uint8_t AUDIO_DISCO_F746NG::OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq) {
    return BSP_AUDIO_OUT_Init(OutputDevice, Volume, AudioFreq);
}

uint8_t AUDIO_DISCO_F746NG::OUT_Play(uint16_t *pBuffer, uint32_t Size) {
    return BSP_AUDIO_OUT_Play(pBuffer, Size);
}

void AUDIO_DISCO_F746NG::OUT_ChangeBuffer(uint16_t *pData, uint16_t Size) {
    BSP_AUDIO_OUT_ChangeBuffer(pData, Size);
}

uint8_t AUDIO_DISCO_F746NG::OUT_Pause(void) {
    return BSP_AUDIO_OUT_Pause();
}

uint8_t AUDIO_DISCO_F746NG::OUT_Resume(void) {
    return BSP_AUDIO_OUT_Resume();
}

uint8_t AUDIO_DISCO_F746NG::OUT_Stop(uint32_t Option) {
    return BSP_AUDIO_OUT_Stop(Option);
}

uint8_t AUDIO_DISCO_F746NG::OUT_SetVolume(uint8_t Volume) {
    return BSP_AUDIO_OUT_SetVolume(Volume);
}

uint8_t AUDIO_DISCO_F746NG::OUT_SetMute(uint32_t Cmd) {
    return BSP_AUDIO_OUT_SetMute(Cmd);
}

uint8_t AUDIO_DISCO_F746NG::OUT_SetOutputMode(uint8_t Output) {
    return BSP_AUDIO_OUT_SetOutputMode(Output);
}

void AUDIO_DISCO_F746NG::OUT_SetFrequency(uint32_t AudioFreq) {
    BSP_AUDIO_OUT_SetFrequency(AudioFreq);
}

void AUDIO_DISCO_F746NG::OUT_SetAudioFrameSlot(uint32_t AudioFrameSlot) {
    BSP_AUDIO_OUT_SetAudioFrameSlot(AudioFrameSlot);
}

void AUDIO_DISCO_F746NG::OUT_DeInit(void) {
    BSP_AUDIO_OUT_DeInit();
}

uint8_t AUDIO_DISCO_F746NG::IN_Init(uint16_t InputDevice, uint8_t Volume, uint32_t AudioFreq) {
    return BSP_AUDIO_IN_Init(InputDevice, Volume, AudioFreq);
}

uint8_t AUDIO_DISCO_F746NG::IN_OUT_Init(uint16_t InputDevice, uint16_t OutputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr) {
    return BSP_AUDIO_IN_OUT_Init(InputDevice, OutputDevice, AudioFreq, BitRes, ChnlNbr);
}

uint8_t  AUDIO_DISCO_F746NG::IN_Record(uint16_t *pbuf, uint32_t size) {
    return BSP_AUDIO_IN_Record(pbuf, size);
}

uint8_t AUDIO_DISCO_F746NG::IN_Stop(uint32_t Option) {
    return BSP_AUDIO_IN_Stop(Option);
}

uint8_t AUDIO_DISCO_F746NG::IN_Pause(void) {
    return BSP_AUDIO_IN_Pause();
}

uint8_t AUDIO_DISCO_F746NG::IN_Resume(void) {
    return BSP_AUDIO_IN_Resume();
}

uint8_t AUDIO_DISCO_F746NG::IN_SetVolume(uint8_t Volume) {
    return BSP_AUDIO_IN_SetVolume(Volume);
}

void AUDIO_DISCO_F746NG::IN_DeInit(void) {
    BSP_AUDIO_IN_DeInit();
}
