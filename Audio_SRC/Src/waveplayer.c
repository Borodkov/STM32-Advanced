/**
  ******************************************************************************
    @file    Audio/Audio_SRC/Src/waveplayer.c
    @author  MCD Application Team
    @brief   This file provides the Audio Out (playback) interface API
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "audio_fw_glo.h"
#include "src236_glo.h"
#include "src441_glo.h"

/* Private define ------------------------------------------------------------*/

/* LCD  RK043FN48H : 480*272 pixels */

#define TOUCH_NEXT_XMIN         325
#define TOUCH_NEXT_XMAX         365
#define TOUCH_NEXT_YMIN         212
#define TOUCH_NEXT_YMAX         252

#define TOUCH_PREVIOUS_XMIN     250
#define TOUCH_PREVIOUS_XMAX     290
#define TOUCH_PREVIOUS_YMIN     212
#define TOUCH_PREVIOUS_YMAX     252

#define TOUCH_STOP_XMIN         170
#define TOUCH_STOP_XMAX         210
#define TOUCH_STOP_YMIN         212
#define TOUCH_STOP_YMAX         252

#define TOUCH_PAUSE_XMIN        100
#define TOUCH_PAUSE_XMAX        124
#define TOUCH_PAUSE_YMIN        212
#define TOUCH_PAUSE_YMAX        252
#define TOUCH_PAUSE_XSPACE      20

#define TOUCH_VOL_MINUS_XMIN    20
#define TOUCH_VOL_MINUS_XMAX    70
#define TOUCH_VOL_MINUS_YMIN    212
#define TOUCH_VOL_MINUS_YMAX    252
#define TOUCH_VOL_MINUS_XOFFSET 4

#define TOUCH_VOL_PLUS_XMIN     402
#define TOUCH_VOL_PLUS_XMAX     452
#define TOUCH_VOL_PLUS_YMIN     212
#define TOUCH_VOL_PLUS_YMAX     252

#define TOUCH_SRC_EN_XMIN      278
#define TOUCH_SRC_EN_XMAX      454
#define TOUCH_SRC_EN_YMIN      55
#define TOUCH_SRC_EN_YMAX      85
#define TOUCH_SRC_EN_XOFFSET   20
#define PLAYER_COUNT_TEXT_XMIN  263
#define MSG_MOD_LINE            15

#define SELECT_SRC_NONE     0
#define SELECT_SRC_236      1
#define SELECT_SRC_441      2

#define CHAR(x) ((x) * (((sFONT *)BSP_LCD_GetFont())->Width))

/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static AUDIO_OUT_BufferTypeDef  BufferCtl;  /* to the codec */
static int16_t FilePos = 0;
static __IO uint32_t uwVolume = 50;
static Point PlayPoints[] = {{TOUCH_STOP_XMIN, TOUCH_STOP_YMIN},
    {TOUCH_STOP_XMAX, (TOUCH_STOP_YMIN + TOUCH_STOP_YMAX) / 2},
    {TOUCH_STOP_XMIN, TOUCH_STOP_YMAX}
};
static Point NextPoints[] = {{TOUCH_NEXT_XMIN, TOUCH_NEXT_YMIN},
    {TOUCH_NEXT_XMAX, (TOUCH_NEXT_YMIN + TOUCH_NEXT_YMAX) / 2},
    {TOUCH_NEXT_XMIN, TOUCH_NEXT_YMAX}
};
static Point PreviousPoints[] = {{TOUCH_PREVIOUS_XMIN, (TOUCH_PREVIOUS_YMIN + TOUCH_PREVIOUS_YMAX) / 2},
    {TOUCH_PREVIOUS_XMAX, TOUCH_PREVIOUS_YMIN},
    {TOUCH_PREVIOUS_XMAX, TOUCH_PREVIOUS_YMAX}
};

static char SRC236_ratio[8][3] = {" 2 ", " 3 ", " 6 ", "1_2", "1_3", "1_6", "3_2", "2_3"};

WAVE_FormatTypeDef WaveFormat;
FIL WavFile;
extern FILELIST_FileTypeDef FileList;

AUDIO_PLAYBACK_CfgTypeDef AudioCfgChange;

/* SRC: Sample Rate Converter variables */
/* WavToSrcTmpBuffer: SRC_FRAME_SIZE (240>147) * 2 (bytes x sample) * 2 (stereo) */
static uint8_t WavToSrcTmpBuffer[SRC236_FRAME_SIZE_MAX * 2 * 2];
static uint8_t SrcTypeSelected; /* 236, 441, None */
static uint8_t SrcIterations;
static uint32_t WavReadSize;    /* nr of bytes to retrieve from USB*/
void *pSrc236PersistentMem;
void *pSrc441PersistentMem;
void *pSrcScratchMem;
static  buffer_t InputBuffer;
static  buffer_t *pInputBuffer = &InputBuffer;
static  buffer_t OutputBuffer;
static  buffer_t *pOutputBuffer = &OutputBuffer;

/* Private function prototypes -----------------------------------------------*/
static AUDIO_ErrorTypeDef GetFileInfo(uint16_t file_idx, WAVE_FormatTypeDef *info);
static uint8_t PlayerInit(uint32_t AudioFreq);
static void AUDIO_PlaybackDisplayButtons(void);
static void AUDIO_AcquireTouchButtons(void);
static void AUDIO_PLAYER_Dynamic_Cfg(void);
static void AUDIO_PLAYER_DisplaySettings(void);
static AUDIO_ErrorTypeDef AUDIO_PLAYER_FromUsbToSrcToCodecBuf(uint16_t offset);

/* Private functions ---------------------------------------------------------*/

/**
    @brief  Initializes Audio Interface.
    @param  None
    @retval Audio error
*/
AUDIO_ErrorTypeDef AUDIO_PLAYER_Init(void) {
    uint32_t src_scratch_mem_size;
    /* Allocat mem for SRC236 and SRC411 */
    pSrc236PersistentMem = malloc(src236_persistent_mem_size);  /* 0x1EC  */
    pSrc441PersistentMem = malloc(src441_persistent_mem_size);  /* 0x0E8  */

    if (src236_scratch_mem_size > src441_scratch_mem_size) {
        src_scratch_mem_size = src236_scratch_mem_size;    /* 0x784 */
    } else {
        src_scratch_mem_size = src441_scratch_mem_size;    /* 0xC9C */
    }

    pSrcScratchMem = malloc(src_scratch_mem_size);       /* 0xC9C */

    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_AUTO, uwVolume, I2S_AUDIOFREQ_48K) == 0) {
        return AUDIO_ERROR_NONE;
    } else {
        return AUDIO_ERROR_IO;
    }
}

/**
    @brief  Starts Audio streaming.
    @param  idx: File index
    @retval Audio error
*/
AUDIO_ErrorTypeDef AUDIO_PLAYER_Start(uint8_t idx) {
    AUDIO_ErrorTypeDef error = AUDIO_ERROR_NONE;
    src236_static_param_t src236_static_param;
    src441_static_param_t src441_static_param;
    uint32_t src_input_frame_size;
    uint32_t src_error;
    f_close(&WavFile);

    if (AUDIO_GetWavObjectNumber() > idx) {
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(0, LINE(3), BSP_LCD_GetXSize() , LINE(1));
        GetFileInfo(idx, &WaveFormat);

        if (WaveFormat.NbrChannels != 2) {
            BSP_LCD_SetTextColor(LCD_COLOR_RED);
            BSP_LCD_DisplayStringAtLine(MSG_MOD_LINE, (uint8_t *) "Error: only stereo audio input is supported. Try another track.");
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            f_close(&WavFile);
            AudioState = AUDIO_STATE_STOP;
            return (AUDIO_ERROR_INVALID_VALUE);
        }

        /* Effect Initialization and configuration */
        /*-----------------------------------------*/
        /* Enables and resets CRC-32 from STM32 HW */
        __HAL_RCC_CRC_CLK_ENABLE();
        CRC->CR = CRC_CR_RESET;

        switch (WaveFormat.SampleRate) {
            case 8000:
                src236_static_param.src_mode = SRC236_RATIO_6;
                SrcTypeSelected = SELECT_SRC_236;
                SrcIterations = 1;
                src_input_frame_size = AUDIO_OUT_BUFFER_SIZE / (8 * SrcIterations * 6);
                break;

            case 16000:
                src236_static_param.src_mode = SRC236_RATIO_3;
                SrcTypeSelected = SELECT_SRC_236;
                SrcIterations = 1;
                src_input_frame_size = AUDIO_OUT_BUFFER_SIZE / (8 * SrcIterations * 3);
                break;

            case 24000:
                src236_static_param.src_mode = SRC236_RATIO_2;
                SrcTypeSelected = SELECT_SRC_236;
                SrcIterations = 1;
                src_input_frame_size = AUDIO_OUT_BUFFER_SIZE / (8 * SrcIterations * 2);
                break;

            case 32000:
                src236_static_param.src_mode = SRC236_RATIO_3_2;
                SrcTypeSelected = SELECT_SRC_236;
                SrcIterations = 2;   /* frame size smaller but processing repeated 2 times */
                src_input_frame_size = AUDIO_OUT_BUFFER_SIZE / (8 * SrcIterations * 3 / 2);
                break;

            case 44100:
                /* src441_static_param does not have params to be configured */
                SrcTypeSelected = SELECT_SRC_441;
                SrcIterations = 3;   /* frame size smaller but processing repeated 3 times */
                src_input_frame_size = (AUDIO_OUT_BUFFER_SIZE / 480) * 441 / (8 * SrcIterations);
                break;

            case 48000:
                SrcTypeSelected = SELECT_SRC_NONE;
                SrcIterations = 2;   /* frame size smaller but processing repeated 2 times considering SRC236 input req.*/
                src_input_frame_size = AUDIO_OUT_BUFFER_SIZE / (8 * SrcIterations); /* half buff, stereo, byte x sample */
                break;

            case 96000:
                src236_static_param.src_mode = SRC236_RATIO_1_2;
                SrcTypeSelected = SELECT_SRC_236;
                SrcIterations = 4;  /* frame size smaller but processing repeated 4 times */
                src_input_frame_size = AUDIO_OUT_BUFFER_SIZE / (8 * SrcIterations * 1 / 2);
                break;

            default:
                BSP_LCD_SetTextColor(LCD_COLOR_RED);
                BSP_LCD_DisplayStringAtLine(MSG_MOD_LINE, (uint8_t *) "Error: This sample frequency is not supported.");
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                f_close(&WavFile);
                AudioState = AUDIO_STATE_STOP;
                return (AUDIO_ERROR_INVALID_VALUE);
        }

        WavReadSize = src_input_frame_size * 4; /* stereo & byte x sample */

        if (SrcTypeSelected == SELECT_SRC_236) {
            /* SRC236 effect reset */
            src_error = src236_reset(pSrc236PersistentMem, pSrcScratchMem);

            if (src_error != SRC236_ERROR_NONE) {
                return (AUDIO_ERROR_SW);
            }

            /* SRC236 effect static parameters setting */
            src_error = src236_setParam(&src236_static_param, pSrc236PersistentMem);

            if (src_error != SRC236_ERROR_NONE) {
                return (AUDIO_ERROR_SW);
            }
        }

        if (SrcTypeSelected == SELECT_SRC_441) {
            /* SRC236 effect reset */
            src_error = src441_reset(pSrc441PersistentMem, pSrcScratchMem);

            if (src_error != SRC441_ERROR_NONE) {
                return (AUDIO_ERROR_SW);
            }

            /* SRC236 effect static parameters setting */
            src_error = src441_setParam(&src441_static_param, pSrc441PersistentMem);

            if (src_error != SRC441_ERROR_NONE) {
                return (AUDIO_ERROR_SW);
            }
        }

        InputBuffer.data_ptr = &WavToSrcTmpBuffer;
        InputBuffer.nb_bytes_per_Sample = WaveFormat.BitPerSample / 8; /* 8 bits in 0ne byte */
        InputBuffer.nb_channels = WaveFormat.NbrChannels;
        InputBuffer.mode = INTERLEAVED;
        InputBuffer.buffer_size = src_input_frame_size;
        OutputBuffer.nb_bytes_per_Sample = WaveFormat.BitPerSample / 8; /* 8 bits in 0ne byte */
        OutputBuffer.nb_channels = WaveFormat.NbrChannels;
        OutputBuffer.mode = INTERLEAVED;
        OutputBuffer.buffer_size = AUDIO_OUT_BUFFER_SIZE / 8; /* half buff, stereo, bytes x sample */

        /* Audio frequency to the codec is always 48Khz for this application*/
        if (PlayerInit(48000) != 0) {
            return AUDIO_ERROR_CODEC;
        }

        BufferCtl.state = BUFFER_OFFSET_NONE;
        BufferCtl.fptr = 0;
        /* Get Data from USB Flash Disk and Remove Wave format header */
        f_lseek(&WavFile, 44);
        /* Fill whole buffer at first time (480+480 stereo samples) */
        error = AUDIO_PLAYER_FromUsbToSrcToCodecBuf(0);

        if (error == AUDIO_ERROR_NONE) {
            error = AUDIO_PLAYER_FromUsbToSrcToCodecBuf(AUDIO_OUT_BUFFER_SIZE / 2);
        }

        /* Sent to codec whole buffer at first time */
        if (error == AUDIO_ERROR_NONE) {
            AudioState = AUDIO_STATE_PLAY;
            AUDIO_PlaybackDisplayButtons();
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(9), (uint8_t *)"[PLAY ]", LEFT_MODE);

            if (BufferCtl.fptr != 0) {
                BSP_AUDIO_OUT_Play((uint16_t *)&BufferCtl.buff[0], AUDIO_OUT_BUFFER_SIZE);
                return AUDIO_ERROR_NONE;
            }
        }
    }

    return AUDIO_ERROR_IO;
}

/**
    @brief  Manages Audio process.
    @param  None
    @retval Audio error
*/
AUDIO_ErrorTypeDef AUDIO_PLAYER_Process(void) {
    uint32_t elapsed_time;
    AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;
    static uint32_t prev_elapsed_time = 0xFFFFFFFF;
    uint8_t str[10];

    switch (AudioState) {
        case AUDIO_STATE_PLAY:
            if (BufferCtl.fptr >= WaveFormat.FileSize) {
                BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
                AudioState = AUDIO_STATE_NEXT;
            }

            if (BufferCtl.state == BUFFER_OFFSET_HALF) {
                audio_error = AUDIO_PLAYER_FromUsbToSrcToCodecBuf(0);

                if (audio_error != AUDIO_ERROR_NONE) {
                    return audio_error;
                }

                /* Dynamic change done when switching buffer */
                /* Related dispay processing takes sometime hence better to do it just after a buffer is refilled */
                AUDIO_PLAYER_Dynamic_Cfg();
                BufferCtl.state = BUFFER_OFFSET_NONE;
            }

            if (BufferCtl.state == BUFFER_OFFSET_FULL) {
                audio_error = AUDIO_PLAYER_FromUsbToSrcToCodecBuf(AUDIO_OUT_BUFFER_SIZE / 2);

                if (audio_error != AUDIO_ERROR_NONE) {
                    return audio_error;
                }

                /* Dynamic change done when switching buffer */
                /* Related dispay processing takes sometime hence better to do it just after a buffer is refilled */
                AUDIO_PLAYER_Dynamic_Cfg();
                BufferCtl.state = BUFFER_OFFSET_NONE;
            }

            /* Display elapsed time */
            elapsed_time = BufferCtl.fptr / WaveFormat.ByteRate;

            if (prev_elapsed_time != elapsed_time) {
                prev_elapsed_time = elapsed_time;
                sprintf((char *)str, "[%02d:%02d]", (int)(elapsed_time / 60), (int)(elapsed_time % 60));
                BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
                BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(8), str, LEFT_MODE);
                BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            }

            /* Update audio state machine according to touch acquisition */
            AUDIO_AcquireTouchButtons();
            break;

        case AUDIO_STATE_START:
            BSP_LCD_FillRect(TOUCH_STOP_XMIN, TOUCH_STOP_YMIN , /* Stop rectangle */
                             TOUCH_STOP_XMAX - TOUCH_STOP_XMIN,
                             TOUCH_STOP_YMAX - TOUCH_STOP_YMIN);
            audio_error = AUDIO_PLAYER_Start(FilePos);

            if (audio_error != AUDIO_ERROR_NONE) {
                return audio_error;
            }

            AUDIO_PLAYER_DisplaySettings();
            AudioState = AUDIO_STATE_PLAY;
            break;

        case AUDIO_STATE_STOP:
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(TOUCH_STOP_XMIN, TOUCH_STOP_YMIN , /* Stop rectangle */
                             TOUCH_STOP_XMAX - TOUCH_STOP_XMIN,
                             TOUCH_STOP_YMAX - TOUCH_STOP_YMIN);
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            BSP_LCD_FillPolygon(PlayPoints, 3);   /* Play icon */
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            sprintf((char *)str, "[00:00]");
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(8), str, LEFT_MODE);
            sprintf((char *)str, "[STOP ]");
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(9), str, LEFT_MODE);
            BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
            AudioState = AUDIO_STATE_WAIT;
            break;

        case AUDIO_STATE_NEXT:
            if (++FilePos >= AUDIO_GetWavObjectNumber()) {
                FilePos = 0;
            }

            BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
            BSP_LCD_DisplayStringAtLine(15, (uint8_t *)"                                                                     ");
            AudioState = AUDIO_STATE_START;
            break;

        case AUDIO_STATE_PREVIOUS:
            if (--FilePos < 0) {
                FilePos = AUDIO_GetWavObjectNumber() - 1;
            }

            BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
            BSP_LCD_DisplayStringAtLine(15, (uint8_t *)"                                                                     ");
            AudioState = AUDIO_STATE_START;
            break;

        case AUDIO_STATE_PAUSE:
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(9), (uint8_t *)"  [PAUSE]", LEFT_MODE);
            BSP_LCD_SetTextColor(LCD_COLOR_RED);    /* Display red pause rectangles */
            BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
            BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
            BSP_AUDIO_OUT_Pause();
            AudioState = AUDIO_STATE_WAIT;
            break;

        case AUDIO_STATE_RESUME:
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(9), (uint8_t *)"  [PLAY ]", LEFT_MODE);
            /* Display blue cyan pause rectangles */
            BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
            BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
            BSP_AUDIO_OUT_Resume();
            AudioState = AUDIO_STATE_PLAY;
            break;

        case AUDIO_STATE_INIT:
            AUDIO_PlaybackDisplayButtons();
            AudioState = AUDIO_STATE_STOP;

        case AUDIO_STATE_WAIT:
        case AUDIO_STATE_IDLE:
        default:
            /* Update audio state machine according to touch acquisition */
            AUDIO_AcquireTouchButtons();
            AUDIO_PLAYER_Dynamic_Cfg();
            break;
    }

    return audio_error;
}

/**
    @brief  Stops Audio streaming.
    @param  None
    @retval Audio error
*/
AUDIO_ErrorTypeDef AUDIO_PLAYER_Stop(void) {
    AudioState = AUDIO_STATE_STOP;
    FilePos = 0;
    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
    f_close(&WavFile);
    return AUDIO_ERROR_NONE;
}

/**
    @brief  Calculates the remaining file size and new position of the pointer.
    @param  None
    @retval None
*/
void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
    if (AudioState == AUDIO_STATE_PLAY) {
        BufferCtl.state = BUFFER_OFFSET_FULL;
    }
}

/**
    @brief  Manages the DMA Half Transfer complete interrupt.
    @param  None
    @retval None
*/
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
    if (AudioState == AUDIO_STATE_PLAY) {
        BufferCtl.state = BUFFER_OFFSET_HALF;
    }
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/

/**
    @brief  Gets the file info.
    @param  file_idx: File index
    @param  info: Pointer to WAV file info
    @retval Audio error
*/
static AUDIO_ErrorTypeDef GetFileInfo(uint16_t file_idx, WAVE_FormatTypeDef *info) {
    uint32_t bytesread;
    uint32_t duration;
    uint8_t str[FILEMGR_FILE_NAME_SIZE + 20];

    if (f_open(&WavFile, (char *)FileList.file[file_idx].name, FA_OPEN_EXISTING | FA_READ) == FR_OK) {
        /* Fill the buffer to Send */
        if (f_read(&WavFile, info, sizeof(WaveFormat), (void *)&bytesread) == FR_OK) {
            BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(3), TOUCH_SRC_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            sprintf((char *)str, "Playing file (%d/%d): %s",
                    file_idx + 1, FileList.ptr,
                    (char *)FileList.file[file_idx].name);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(4), TOUCH_SRC_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAtLine(4, str);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(5), TOUCH_SRC_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            sprintf((char *)str,  "Sample rate : %d Hz", (int)(info->SampleRate));
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(6), TOUCH_SRC_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAtLine(6, str);
            sprintf((char *)str,  "Channels number : %d", info->NbrChannels);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(7), TOUCH_SRC_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAtLine(7, str);
            duration = info->FileSize / info->ByteRate;
            sprintf((char *)str, "File Size : %d KB [%02d:%02d]", (int)(info->FileSize / 1024), (int)(duration / 60), (int)(duration % 60));
            BSP_LCD_ClearStringLine(8);
            BSP_LCD_DisplayStringAtLine(8, str);
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(8), (uint8_t *)"[00:00]", LEFT_MODE);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            sprintf((char *)str,  "Volume : %lu", uwVolume);
            BSP_LCD_ClearStringLine(9);
            BSP_LCD_DisplayStringAtLine(9, str);
            /*clear next lines*/
            BSP_LCD_ClearStringLine(10);
            BSP_LCD_ClearStringLine(11);
            BSP_LCD_ClearStringLine(12);
            BSP_LCD_ClearStringLine(13);
            BSP_LCD_ClearStringLine(14);
            BSP_LCD_ClearStringLine(15);
            BSP_LCD_ClearStringLine(16);
            return AUDIO_ERROR_NONE;
        }

        f_close(&WavFile);
    }

    return AUDIO_ERROR_IO;
}

/**
    @brief  Initializes the Wave player.
    @param  AudioFreq: Audio sampling frequency
    @retval None
*/
static uint8_t PlayerInit(uint32_t AudioFreq) {
    /* Initialize the Audio codec and all related peripherals (I2S, I2C, IOExpander, IOs...) */
    if (BSP_AUDIO_OUT_Init(OUTPUT_DEVICE_BOTH, uwVolume, AudioFreq) != 0) {
        return 1;
    } else {
        BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
        return 0;
    }
}

/**
    @brief  Display interface touch screen buttons
    @param  None
    @retval None
*/
static void AUDIO_PlaybackDisplayButtons(void) {
    BSP_LCD_SetFont(&LCD_LOG_HEADER_FONT);
    BSP_LCD_ClearStringLine(13);            /* Clear dedicated zone */
    BSP_LCD_ClearStringLine(14);
    BSP_LCD_ClearStringLine(15);
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
    BSP_LCD_FillPolygon(PreviousPoints, 3);   /* Previous track icon */
    BSP_LCD_FillRect(TOUCH_PREVIOUS_XMIN, TOUCH_PREVIOUS_YMIN , 10, TOUCH_PREVIOUS_YMAX - TOUCH_PREVIOUS_YMIN);
    BSP_LCD_FillPolygon(NextPoints, 3);       /* Next track icon */
    BSP_LCD_FillRect(TOUCH_NEXT_XMAX - 9, TOUCH_NEXT_YMIN , 10, TOUCH_NEXT_YMAX - TOUCH_NEXT_YMIN);
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);    /* Pause rectangles */
    BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
    BSP_LCD_FillRect(TOUCH_STOP_XMIN, TOUCH_STOP_YMIN , /* Stop rectangle */
                     TOUCH_STOP_XMAX - TOUCH_STOP_XMIN,
                     TOUCH_STOP_YMAX - TOUCH_STOP_YMIN);
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_DrawRect(TOUCH_VOL_MINUS_XMIN, TOUCH_VOL_MINUS_YMIN , /* VOl- rectangle */
                     TOUCH_VOL_MINUS_XMAX - TOUCH_VOL_MINUS_XMIN,
                     TOUCH_VOL_MINUS_YMAX - TOUCH_VOL_MINUS_YMIN);
    BSP_LCD_DisplayStringAt(TOUCH_VOL_MINUS_XMIN + TOUCH_VOL_MINUS_XOFFSET, LINE(14), (uint8_t *)"VOl-", LEFT_MODE);
    BSP_LCD_DrawRect(TOUCH_VOL_PLUS_XMIN, TOUCH_VOL_PLUS_YMIN , /* VOl+ rectangle */
                     TOUCH_VOL_PLUS_XMAX - TOUCH_VOL_PLUS_XMIN,
                     TOUCH_VOL_PLUS_YMAX - TOUCH_VOL_PLUS_YMIN);
    BSP_LCD_DisplayStringAt(TOUCH_VOL_PLUS_XMIN + TOUCH_VOL_MINUS_XOFFSET, LINE(14), (uint8_t *)"VOl+", LEFT_MODE);
    AUDIO_PLAYER_DisplaySettings();
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
}

/**
    @brief  Detect touch screen state and modify audio state machine accordingly
    @param  None
    @retval None
*/
static void AUDIO_AcquireTouchButtons(void) {
    static TS_StateTypeDef  TS_State = {0};

    if (TS_State.touchDetected == 1) { /* If previous touch has not been released, we don't proceed any touch command */
        BSP_TS_GetState(&TS_State);
    } else {
        BSP_TS_GetState(&TS_State);

        if (TS_State.touchDetected == 1) {
            if ((TS_State.touchX[0] > TOUCH_PAUSE_XMIN) && (TS_State.touchX[0] < TOUCH_PAUSE_XMAX) &&
                    (TS_State.touchY[0] > TOUCH_PAUSE_YMIN) && (TS_State.touchY[0] < TOUCH_PAUSE_YMAX)) {
                if (AudioState == AUDIO_STATE_PLAY) {
                    AudioState = AUDIO_STATE_PAUSE;
                } else {
                    AudioState = AUDIO_STATE_RESUME;
                }
            } else if ((TS_State.touchX[0] > TOUCH_NEXT_XMIN) && (TS_State.touchX[0] < TOUCH_NEXT_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_NEXT_YMIN) && (TS_State.touchY[0] < TOUCH_NEXT_YMAX)) {
                AudioState = AUDIO_STATE_NEXT;
            } else if ((TS_State.touchX[0] > TOUCH_PREVIOUS_XMIN) && (TS_State.touchX[0] < TOUCH_PREVIOUS_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_PREVIOUS_YMIN) && (TS_State.touchY[0] < TOUCH_PREVIOUS_YMAX)) {
                AudioState = AUDIO_STATE_PREVIOUS;
            } else if ((TS_State.touchX[0] > TOUCH_STOP_XMIN) && (TS_State.touchX[0] < TOUCH_STOP_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_STOP_YMIN) && (TS_State.touchY[0] < TOUCH_STOP_YMAX)) {
                if (AudioState == AUDIO_STATE_PLAY) {
                    AudioState = AUDIO_STATE_STOP;
                } else {
                    AudioState = AUDIO_STATE_START;
                }
            } else if ((TS_State.touchX[0] > TOUCH_VOL_MINUS_XMIN) && (TS_State.touchX[0] < TOUCH_VOL_MINUS_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_VOL_MINUS_YMIN) && (TS_State.touchY[0] < TOUCH_VOL_MINUS_YMAX)) {
                AudioCfgChange = AUDIO_CFG_VOLUME_DOWN;
            } else if ((TS_State.touchX[0] > TOUCH_VOL_PLUS_XMIN) && (TS_State.touchX[0] < TOUCH_VOL_PLUS_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_VOL_PLUS_YMIN) && (TS_State.touchY[0] < TOUCH_VOL_PLUS_YMAX)) {
                AudioCfgChange = AUDIO_CFG_VOLUME_UP;
            }
        }
    }
}


/**
    @brief  Manages Audio dynamic changes during playback (e.g. volume up/down).
    @param  None
    @retval None
*/
static void AUDIO_PLAYER_Dynamic_Cfg(void) {
    uint8_t str[13];

    switch (AudioCfgChange) {
        case AUDIO_CFG_VOLUME_UP:
            if (((AudioState == AUDIO_STATE_WAIT) || (AudioState == AUDIO_STATE_IDLE)) && (FilePos == 0)) {
                /*clear list lines*/
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(0, LINE(4), PLAYER_COUNT_TEXT_XMIN, LINE(16) - LINE(4));
                AUDIO_PlaybackDisplayButtons();
            }

            if (uwVolume <= 90) {
                uwVolume += 10;
            }

            BSP_AUDIO_OUT_SetVolume(uwVolume);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            sprintf((char *)str,  "Volume : %lu ", uwVolume);
            BSP_LCD_DisplayStringAtLine(9, str);
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_VOLUME_DOWN:
            if (((AudioState == AUDIO_STATE_WAIT) || (AudioState == AUDIO_STATE_IDLE)) && (FilePos == 0)) {
                /*clear list lines*/
                BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
                BSP_LCD_FillRect(0, LINE(4), PLAYER_COUNT_TEXT_XMIN, LINE(16) - LINE(4));
                AUDIO_PlaybackDisplayButtons();
            }

            if (uwVolume >= 10) {
                uwVolume -= 10;
            }

            BSP_AUDIO_OUT_SetVolume(uwVolume);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            sprintf((char *)str,  "Volume : %lu ", uwVolume);
            BSP_LCD_DisplayStringAtLine(9, str);
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_NONE:
        default:
            break;
    }
}

/**
    @brief  Restore display text color and font.
    @param  None
    @retval None
*/
static void AUDIO_PLAYER_DisplaySettings(void) {
    uint8_t str[30];
    src236_static_param_t src236_static_param;
    char *rate;
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
    BSP_LCD_ClearStringLine(MSG_MOD_LINE);

    switch (SrcTypeSelected) {
        case SELECT_SRC_NONE:
            sprintf((char *)str, "SRC: [NONE]");
            break;

        case SELECT_SRC_236:
            src236_getParam(&src236_static_param, pSrc236PersistentMem);
            rate = SRC236_ratio[src236_static_param.src_mode];
            sprintf((char *)str, "SRC: [SRC236, rate=%c%c%c]", rate[0], rate[1], rate[2]);
            break;

        case SELECT_SRC_441:
            sprintf((char *)str, "SRC: [SRC441]");
            break;
    }

    BSP_LCD_DisplayStringAt(TOUCH_NEXT_XMIN - 10, LINE(MSG_MOD_LINE), (uint8_t *)str, LEFT_MODE);
}

/**
    @brief  Read from USB a nr of samples such that after SRC conversion
            480 stereo samples are provided to the codec each 10 ms
    @param  bytesread: returns to the caller the bytes read from USB
    @param  offset: first flip flop buffer or second flip flop buffer
    @retval error: error type (default AUDIO_ERROR_NONE)
*/
static AUDIO_ErrorTypeDef AUDIO_PLAYER_FromUsbToSrcToCodecBuf(uint16_t offset) {
    uint32_t bytesread;
    uint32_t src_error;
    uint16_t i;

    if (SrcTypeSelected == SELECT_SRC_NONE) {
        if (f_read(&WavFile,
                   &BufferCtl.buff[offset],
                   AUDIO_OUT_BUFFER_SIZE / 2,
                   (void *)&bytesread) != FR_OK) {
            return (AUDIO_ERROR_IO);
        }

        BufferCtl.fptr += bytesread;
    } else { /* SrcTypeSelected != NONE */
        for (i = 0; i < SrcIterations; ++i) {
            if (f_read(&WavFile,
                       &WavToSrcTmpBuffer,
                       WavReadSize,
                       (void *)&bytesread) == FR_OK) {
                BufferCtl.fptr += bytesread;
                OutputBuffer.data_ptr = &BufferCtl.buff[offset + i * AUDIO_OUT_BUFFER_SIZE / (2 * SrcIterations)];

                if (SrcTypeSelected == SELECT_SRC_236) {
                    src_error = src236_process(pInputBuffer, pOutputBuffer, pSrc236PersistentMem);

                    if (src_error != SRC236_ERROR_NONE) {
                        return (AUDIO_ERROR_SW);
                    }
                } else { /* SrcTypeSelected == 441 */
                    src_error = src441_process(pInputBuffer, pOutputBuffer, pSrc441PersistentMem);

                    if (src_error != SRC441_ERROR_NONE) {
                        return (AUDIO_ERROR_SW);
                    }
                }
            } else {
                return (AUDIO_ERROR_IO);
            }
        }
    }

    return AUDIO_ERROR_NONE;
}
