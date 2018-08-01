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

#ifndef __AUDIO_DISCO_F746NG_H
#define __AUDIO_DISCO_F746NG_H

#include "stm32746g_discovery_audio.h"

/*
    This class drives the uSD card driver mounted on DISCO_F746NG board.

    Usage:

    #include "mbed.h"
    #include "AUDIO_DISCO_F746NG.h"

    AUDIO_DISCO_F746NG audio;

    int main()
    {
      audio.IN_OUT_Init(INPUT_DEVICE_DIGITAL_MICROPHONE_2, OUTPUT_DEVICE_HEADPHONE, 90, DEFAULT_AUDIO_IN_FREQ):
      audio.IN_Record((uint16_t*)AUDIO_BUFFER_IN, AUDIO_BLOCK_SIZE);
      audio.OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
      audio.OUT_Play((uint16_t*)AUDIO_BUFFER_OUT, AUDIO_BLOCK_SIZE * 2);
      while 1 {}

    }
*/
class AUDIO_DISCO_F746NG {

  public:
    //! Constructor
    AUDIO_DISCO_F746NG();

    //! Destructor
    ~AUDIO_DISCO_F746NG();

    /**
        @brief  Configures the audio peripherals.
        @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
                              or OUTPUT_DEVICE_BOTH.
        @param  Volume: Initial volume level (from 0 (Mute); to 100 (Max))
        @param  AudioFreq: Audio frequency used to play the audio stream.
        @note   The I2S PLL input clock must be done in the user application.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t OUT_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);

    /**
        @brief  Starts playing audio stream from a data buffer for a determined size.
        @param  pBuffer: Pointer to the buffer
        @param  Size: Number of audio data in BYTES unit.
                In memory, first element is for left channel, second element is for right channel
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t OUT_Play(uint16_t *pBuffer, uint32_t Size);

    /**
        @brief  Sends n-Bytes on the SAI interface.
        @param  pData: pointer on data address
        @param  Size: number of data to be written
        @retval None
    */
    void OUT_ChangeBuffer(uint16_t *pData, uint16_t Size);

    /**
        @brief  This function Pauses the audio file stream. In case
                of using DMA, the DMA Pause feature is used.
        @note When calling OUT_Pause(); function for pause, only
                 OUT_Resume(); function should be called for resume (use of BSP_AUDIO_OUT_Play()
                 function for resume could lead to unexpected behaviour);.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t OUT_Pause(void);

    /**
        @brief  This function  Resumes the audio file stream.
        @note When calling OUT_Pause(); function for pause, only
                 OUT_Resume(); function should be called for resume (use of BSP_AUDIO_OUT_Play()
                 function for resume could lead to unexpected behaviour);.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t OUT_Resume(void);

    /**
        @brief  Stops audio playing and Power down the Audio Codec.
        @param  Option: could be one of the following parameters
                  - CODEC_PDWN_SW: for software power off (by writing registers);.
                                   Then no need to reconfigure the Codec after power on.
                  - CODEC_PDWN_HW: completely shut down the codec (physically);.
                                   Then need to reconfigure the Codec after power on.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t OUT_Stop(uint32_t Option);

    /**
        @brief  Controls the current audio volume level.
        @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for
                Mute and 100 for Max volume level);.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t OUT_SetVolume(uint8_t Volume);

    /**
        @brief  Enables or disables the MUTE mode by software
        @param  Cmd: Could be AUDIO_MUTE_ON to mute sound or AUDIO_MUTE_OFF to
                unmute the codec and restore previous volume level.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t OUT_SetMute(uint32_t Cmd);

    /**
        @brief  Switch dynamically (while audio file is played); the output target
                (speaker or headphone);.
        @param  Output: The audio output target: OUTPUT_DEVICE_SPEAKER,
                OUTPUT_DEVICE_HEADPHONE or OUTPUT_DEVICE_BOTH
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t OUT_SetOutputMode(uint8_t Output);

    /**
        @brief  Updates the audio frequency.
        @param  AudioFreq: Audio frequency used to play the audio stream.
        @note   This API should be called after the OUT_Init(); to adjust the
                audio frequency.
        @retval None
    */
    void OUT_SetFrequency(uint32_t AudioFreq);

    /**
        @brief  Updates the Audio frame slot configuration.
        @param  AudioFrameSlot: specifies the audio Frame slot
                This parameter can be one of the following values
                   @arg CODEC_AUDIOFRAME_SLOT_0123
                   @arg CODEC_AUDIOFRAME_SLOT_02
                   @arg CODEC_AUDIOFRAME_SLOT_13
        @note   This API should be called after the OUT_Init(); to adjust the
                audio frame slot.
        @retval None
    */
    void OUT_SetAudioFrameSlot(uint32_t AudioFrameSlot);

    /**
        @brief  Deinit the audio peripherals.
        @retval None
    */
    void OUT_DeInit(void);

    /**
        @brief  Tx Transfer completed callbacks.
        @param  hsai: SAI handle
        @retval None
    */
    void HAL_SAI_TxCpltCallback(SAI_HandleTypeDef *hsai);

    /**
        @brief  Tx Half Transfer completed callbacks.
        @param  hsai: SAI handle
        @retval None
    */
    void HAL_SAI_TxHalfCpltCallback(SAI_HandleTypeDef *hsai);

    /**
        @brief  SAI error callbacks.
        @param  hsai: SAI handle
        @retval None
    */
    void HAL_SAI_ErrorCallback(SAI_HandleTypeDef *hsai);

    /**
        @brief  Initializes the output Audio Codec audio interface (SAI);.
        @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
        @note   The default SlotActive configuration is set to CODEC_AUDIOFRAME_SLOT_0123
                and user can update this configuration using
        @retval None
    */
    static void SAIx_Out_Init(uint32_t AudioFreq);

    /**
        @brief  Deinitializes the output Audio Codec audio interface (SAI);.
        @retval None
    */
    static void SAIx_Out_DeInit(void);

    /**
        @brief  Initializes wave recording.
        @param  InputDevice: INPUT_DEVICE_DIGITAL_MICROPHONE_2 or INPUT_DEVICE_INPUT_LINE_1
        @param  Volume: Initial volume level (in range 0(Mute);..80(+0dB)..100(+17.625dB))
        @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t IN_Init(uint16_t InputDevice, uint8_t Volume, uint32_t AudioFreq);

    /**
        @brief  Initializes wave recording and playback in parallel.
        @param  InputDevice: INPUT_DEVICE_DIGITAL_MICROPHONE_2
        @param  OutputDevice: OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
                              or OUTPUT_DEVICE_BOTH.
        @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
        @param  BitRes: Audio frequency to be configured.
        @param  ChnlNbr: Channel number.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t IN_OUT_Init(uint16_t InputDevice, uint16_t OutputDevice, uint32_t AudioFreq, uint32_t BitRes, uint32_t ChnlNbr);

    /**
        @brief  Starts audio recording.
        @param  pbuf: Main buffer pointer for the recorded data storing
        @param  size: size of the recorded buffer in number of elements (typically number of half-words);
                      Be careful that it is not the same unit than OUT_Play function
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t  IN_Record(uint16_t *pbuf, uint32_t size);

    /**
        @brief  Stops audio recording.
        @param  Option: could be one of the following parameters
                  - CODEC_PDWN_SW: for software power off (by writing registers);.
                                   Then no need to reconfigure the Codec after power on.
                  - CODEC_PDWN_HW: completely shut down the codec (physically);.
                                   Then need to reconfigure the Codec after power on.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t IN_Stop(uint32_t Option);

    /**
        @brief  Pauses the audio file stream.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t IN_Pause(void);

    /**
        @brief  Resumes the audio file stream.
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t IN_Resume(void);

    /**
        @brief  Controls the audio in volume level.
        @param  Volume: Volume level in range 0(Mute);..80(+0dB)..100(+17.625dB)
        @retval AUDIO_OK if correct communication, else wrong communication
    */
    uint8_t IN_SetVolume(uint8_t Volume);

    /**
        @brief  Deinit the audio IN peripherals.
        @retval None
    */
    void IN_DeInit(void);

    /**
        @brief  Rx Transfer completed callbacks.
        @param  hsai: SAI handle
        @retval None
    */
    void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai);

    /**
        @brief  Rx Half Transfer completed callbacks.
        @param  hsai: SAI handle
        @retval None
    */
    void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai);

    /**
        @brief  Initializes the input Audio Codec audio interface (SAI);.
        @param  SaiOutMode: SAI_MODEMASTER_TX (for record and playback in parallel);
                            or SAI_MODEMASTER_RX (for record only);.
        @param  SlotActive: CODEC_AUDIOFRAME_SLOT_02 or CODEC_AUDIOFRAME_SLOT_13
        @param  AudioFreq: Audio frequency to be configured for the SAI peripheral.
        @retval None
    */
    static void SAIx_In_Init(uint32_t SaiOutMode, uint32_t SlotActive, uint32_t AudioFreq);

    /**
        @brief  Deinitializes the output Audio Codec audio interface (SAI);.
        @retval None
    */
    static void SAIx_In_DeInit(void);

  private:

};

#endif
