
/**
  ******************************************************************************
    @file    Audio/Audio_SMR/Src/waveplayer.c
    @author  MCD Application Team
    @brief   This file provides the Audio Out (playback) interface API
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "audio_fw_glo.h"
#include "smr_glo.h"

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
#define TOUCH_PAUSE_XMAX        135
#define TOUCH_PAUSE_YMIN        212
#define TOUCH_PAUSE_YMAX        252
#define TOUCH_PAUSE_XSPACE      20

#define TOUCH_VOL_MINUS_XMIN    20
#define TOUCH_VOL_MINUS_XMAX    70
#define TOUCH_VOL_MINUS_YMIN    212
#define TOUCH_VOL_MINUS_YMAX    252
#define TOUCH_VOL_MINUS_XOFFSET 4
#define TOUCH_VOL_LINE          14

#define TOUCH_VOL_PLUS_XMIN     402
#define TOUCH_VOL_PLUS_XMAX     452
#define TOUCH_VOL_PLUS_YMIN     212
#define TOUCH_VOL_PLUS_YMAX     252

#define TOUCH_SMR_EN_XMIN      278
#define TOUCH_SMR_EN_XMAX      454
#define TOUCH_SMR_EN_YMIN      55
#define TOUCH_SMR_EN_YMAX      85
#define TOUCH_SMR_EN_XOFFSET   20

#define TOUCH_SMR_NEXT_AVGTIME_XMIN    412
#define TOUCH_SMR_NEXT_AVGTIME_XMAX    452
#define TOUCH_SMR_NEXT_AVGTIME_YMIN    130
#define TOUCH_SMR_NEXT_AVGTIME_YMAX    170

#define TOUCH_SMR_PREVIOUS_AVGTIME_XMIN    342
#define TOUCH_SMR_PREVIOUS_AVGTIME_XMAX    382
#define TOUCH_SMR_PREVIOUS_AVGTIME_YMIN    130
#define TOUCH_SMR_PREVIOUS_AVGTIME_YMAX    170

#define PLAYER_COUNT_TEXT_XMIN  263
#define PLAYER_COUNT_TEXT_LINE   8
#define MSG_MOD_LINE            15

#define CHAR(x) ((x) * (((sFONT *)BSP_LCD_GetFont())->Width))


/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static AUDIO_OUT_BufferTypeDef  BufferCtl;
static int16_t FilePos = 0;

static int16_t SmrEn = 1;
static int16_t SmrAvgTime = AUDIO_CFG_SMR_AVGTIME_DEFAULT;


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


WAVE_FormatTypeDef WaveFormat;
FIL WavFile;
extern FILELIST_FileTypeDef FileList;

AUDIO_PLAYBACK_CfgTypeDef AudioCfgChange;

/* SMR Manager */
void *pSmrPersistentMem = NULL;
void *pSmrScratchMem = NULL;

static  smr_static_param_t smr_static_param;
static  smr_dynamic_param_t smr_dynamic_param;

static  buffer_t BufferHandler;
static  buffer_t *pBufferHandler = &BufferHandler;

#ifdef DISPLAY_OUTPUT
static  uint8_t str_pwr[100];
#endif

/* Private function prototypes -----------------------------------------------*/
static AUDIO_ErrorTypeDef GetFileInfo(uint16_t file_idx, WAVE_FormatTypeDef *info);
static uint8_t PlayerInit(uint32_t AudioFreq);
static void AUDIO_PlaybackDisplayButtons(void);
static void AUDIO_AcquireTouchButtons(void);
static int32_t  AUDIO_PLAYER_Dynamic_Cfg(void);
static void AUDIO_DisplaySmrStaticSettings(void);
static void AUDIO_DisplaySmrSettings(void);

/* Private functions ---------------------------------------------------------*/

/**
    @brief  Initializes Audio Interface.
    @param  None
    @retval Audio error
*/
AUDIO_ErrorTypeDef AUDIO_PLAYER_Init(void) {
    /* Allocat mem for Smr */
    pSmrPersistentMem = malloc(smr_persistent_mem_size); /* smr_persistent_mem_size  0x188 */
    pSmrScratchMem = malloc(smr_scratch_mem_size);       /* smr_scratch_mem_size  0xF04 */

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
    uint32_t bytesread;
    int32_t error = SMR_ERROR_NONE;
    f_close(&WavFile);

    if (AUDIO_GetWavObjectNumber() > idx) {
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(0, LINE(3), BSP_LCD_GetXSize() , LINE(1));
        GetFileInfo(idx, &WaveFormat);

        if (WaveFormat.SampleRate != 48000) {
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(TOUCH_SMR_EN_XMIN, TOUCH_SMR_EN_YMIN, TOUCH_SMR_EN_XMAX - TOUCH_SMR_EN_XMIN + 2, TOUCH_SMR_EN_YMAX - TOUCH_SMR_EN_YMIN + 2);
            BSP_LCD_SetTextColor(LCD_COLOR_RED);
            BSP_LCD_DisplayStringAtLine(MSG_MOD_LINE, (uint8_t *) "Error: This sample frequency is not supported. Try another track.");
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            f_close(&WavFile);
            AudioState = AUDIO_STATE_STOP;
            return (AUDIO_ERROR_INVALID_VALUE);
        }

        if (WaveFormat.NbrChannels != 2) {
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(TOUCH_SMR_EN_XMIN, TOUCH_SMR_EN_YMIN, TOUCH_SMR_EN_XMAX - TOUCH_SMR_EN_XMIN + 2, TOUCH_SMR_EN_YMAX - TOUCH_SMR_EN_YMIN + 2);
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
        /* SMR effect reset */
        error = smr_reset(pSmrPersistentMem, pSmrScratchMem);

        if (error != SMR_ERROR_NONE) {
            return (AUDIO_ERROR_SW);
        }

        /* SMR effect static parameters setting */
        smr_static_param.sampling_rate = WaveFormat.SampleRate;   /* only sampling rate supported */
        error = smr_setParam(&smr_static_param, pSmrPersistentMem);

        if (error != SMR_ERROR_NONE) {
            return (AUDIO_ERROR_SW);
        }

        /* SMR dynamic parameters that can be updated here every frame if required */
        smr_dynamic_param.enable = 1;                        /* SMR module enabler */
        smr_dynamic_param.averaging_time = AUDIO_CFG_SMR_AVGTIME_DEFAULT;
        smr_dynamic_param.filter_type = SMR_PREFILTER_AWEIGHTING;
        error = smr_setConfig(&smr_dynamic_param, pSmrPersistentMem);

        if (error != SMR_ERROR_NONE) {
            return (AUDIO_ERROR_SW);
        }

        /* I/O buffers settings */
        BufferHandler.nb_bytes_per_Sample = WaveFormat.BitPerSample / 8; /* 8 bits in 0ne byte */
        BufferHandler.nb_channels = 2; /* stereo */
        BufferHandler.buffer_size = AUDIO_OUT_BUFFER_SIZE / (2 * 4); /* just half buffer is process (size per channel) */
        BufferHandler.mode = INTERLEAVED;
        /*Adjust the Audio frequency */
        PlayerInit(WaveFormat.SampleRate);
        BufferCtl.state = BUFFER_OFFSET_NONE;
        /* Get Data from USB Flash Disk and Remove Wave format header */
        f_lseek(&WavFile, 44);

        /* Fill whole buffer at first time */
        if (f_read(&WavFile,
                   &BufferCtl.buff[0],
                   AUDIO_OUT_BUFFER_SIZE,
                   (void *)&bytesread) == FR_OK) {
            AudioState = AUDIO_STATE_PLAY;
            AUDIO_PlaybackDisplayButtons();
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_LINE + 1), (uint8_t *)"[PLAY ]", LEFT_MODE);
            {
                if (bytesread != 0) {
                    BSP_AUDIO_OUT_Play((uint16_t *)&BufferCtl.buff[0], AUDIO_OUT_BUFFER_SIZE);
                    BufferCtl.fptr = bytesread;
                    return AUDIO_ERROR_NONE;
                }
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
    uint32_t bytesread, elapsed_time;
    AUDIO_ErrorTypeDef audio_error = AUDIO_ERROR_NONE;
    static uint32_t prev_elapsed_time = 0xFFFFFFFF;
    uint8_t str[10];
    int32_t error = SMR_ERROR_NONE;

    switch (AudioState) {
        case AUDIO_STATE_PLAY:
            if (BufferCtl.fptr >= WaveFormat.FileSize) {
                BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
                AudioState = AUDIO_STATE_NEXT;
            }

            if (BufferCtl.state == BUFFER_OFFSET_HALF) {
                if (f_read(&WavFile,
                           &BufferCtl.buff[0],
                           AUDIO_OUT_BUFFER_SIZE / 2,
                           (void *)&bytesread) != FR_OK) {
                    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
                    return AUDIO_ERROR_IO;
                }

                BufferHandler.data_ptr = &BufferCtl.buff[0];
                error = smr_process(pBufferHandler, pBufferHandler, pSmrPersistentMem);

                if (error != SMR_ERROR_NONE) {
                    return (AUDIO_ERROR_SW);
                }

                /* output signal power sent to terminal IO */
                error = smr_getConfig(&smr_dynamic_param, pSmrPersistentMem);

                if (error != SMR_ERROR_NONE) {
                    return AUDIO_ERROR_SW;
                }

#ifdef DISPLAY_OUTPUT
                BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
                sprintf((char *)str_pwr, "Left mean power:  %ld dB", (int32_t)((float)smr_dynamic_param.mean_level_left * 0.25));
                BSP_LCD_DisplayStringAt(CHAR(0), LINE(10), (uint8_t *)str_pwr, LEFT_MODE);
                sprintf((char *)str_pwr, "Right mean power:  %ld dB", (int32_t)((float)smr_dynamic_param.mean_level_right * 0.25));
                BSP_LCD_DisplayStringAt(CHAR(0), LINE(11), (uint8_t *)str_pwr, LEFT_MODE);
#endif
                /* Dynamic change done when switching buffer */
                /* Related dispay processing takes sometime hence better to do it just after a buffer is refilled */
                error = AUDIO_PLAYER_Dynamic_Cfg();

                if (error != SMR_ERROR_NONE) {
                    return (AUDIO_ERROR_INVALID_VALUE);
                }

                BufferCtl.state = BUFFER_OFFSET_NONE;
                BufferCtl.fptr += bytesread;
            }

            if (BufferCtl.state == BUFFER_OFFSET_FULL) {
                if (f_read(&WavFile,
                           &BufferCtl.buff[AUDIO_OUT_BUFFER_SIZE / 2],
                           AUDIO_OUT_BUFFER_SIZE / 2,
                           (void *)&bytesread) != FR_OK) {
                    BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
                    return AUDIO_ERROR_IO;
                }

                BufferHandler.data_ptr = &BufferCtl.buff[AUDIO_OUT_BUFFER_SIZE / 2];
                error = smr_process(pBufferHandler, pBufferHandler, pSmrPersistentMem);

                if (error != SMR_ERROR_NONE) {
                    return (AUDIO_ERROR_SW);
                }

                /* output signal power sent to terminal IO */
                error = smr_getConfig(&smr_dynamic_param, pSmrPersistentMem);

                if (error != SMR_ERROR_NONE) {
                    return AUDIO_ERROR_SW;
                }

#ifdef DISPLAY_OUTPUT
                BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
                sprintf((char *)str_pwr, "Left mean power:  %ld dB", (int32_t)((float)smr_dynamic_param.mean_level_left * 0.25));
                BSP_LCD_DisplayStringAt(CHAR(0), LINE(10), (uint8_t *)str_pwr, LEFT_MODE);
                sprintf((char *)str_pwr, "Right mean power:  %ld dB", (int32_t)((float)smr_dynamic_param.mean_level_right * 0.25));
                BSP_LCD_DisplayStringAt(CHAR(0), LINE(11), (uint8_t *)str_pwr, LEFT_MODE);
#endif
                /* Dynamic change done when switching buffer */
                /* Related dispay processing takes sometime hence better to do it just after a buffer is refilled */
                error = AUDIO_PLAYER_Dynamic_Cfg();

                if (error != SMR_ERROR_NONE) {
                    return (AUDIO_ERROR_INVALID_VALUE);
                }

                BufferCtl.state = BUFFER_OFFSET_NONE;
                BufferCtl.fptr += bytesread;
            }

            /* Display elapsed time */
            elapsed_time = BufferCtl.fptr / WaveFormat.ByteRate;

            if (prev_elapsed_time != elapsed_time) {
                prev_elapsed_time = elapsed_time;
                sprintf((char *)str, "[%02d:%02d]", (int)(elapsed_time / 60), (int)(elapsed_time % 60));
                BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
                BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_LINE), str, LEFT_MODE);
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

            /* SMR dynamic parameters that can be updated here every frame if required */
            smr_getConfig(&smr_dynamic_param, pSmrPersistentMem);
            smr_dynamic_param.enable = SmrEn;               /* SMR module enabler */
            smr_dynamic_param.averaging_time = SmrAvgTime;
            error = smr_setConfig(&smr_dynamic_param, pSmrPersistentMem);

            if (error != SMR_ERROR_NONE) {
                return (AUDIO_ERROR_SW);
            }

            AUDIO_DisplaySmrStaticSettings();
            AUDIO_DisplaySmrSettings();
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
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_LINE), str, LEFT_MODE);
            sprintf((char *)str, "[STOP ]");
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_LINE + 1), str, LEFT_MODE);
            BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
            AudioState = AUDIO_STATE_WAIT;
            break;

        case AUDIO_STATE_NEXT:
            if (++FilePos >= AUDIO_GetWavObjectNumber()) {
                FilePos = 0;
            }

            BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
            AUDIO_PLAYER_Start(FilePos);
            break;

        case AUDIO_STATE_PREVIOUS:
            if (--FilePos < 0) {
                FilePos = AUDIO_GetWavObjectNumber() - 1;
            }

            BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
            AUDIO_PLAYER_Start(FilePos);
            break;

        case AUDIO_STATE_PAUSE:
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_LINE + 1), (uint8_t *)"[PAUSE]", LEFT_MODE);
            BSP_LCD_SetTextColor(LCD_COLOR_RED);    /* Display red pause rectangles */
            BSP_LCD_FillRect(TOUCH_PAUSE_XMIN, TOUCH_PAUSE_YMIN , 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
            BSP_LCD_FillRect(TOUCH_PAUSE_XMIN + TOUCH_PAUSE_XSPACE, TOUCH_PAUSE_YMIN, 15, TOUCH_PAUSE_YMAX - TOUCH_PAUSE_YMIN);
            BSP_AUDIO_OUT_Pause();
            AudioState = AUDIO_STATE_WAIT;
            break;

        case AUDIO_STATE_RESUME:
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_LINE + 1), (uint8_t *)"[PLAY ]", LEFT_MODE);
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
            /* Define default dynamic parameters at init*/
            smr_dynamic_param.filter_type = SMR_PREFILTER_AWEIGHTING;
            smr_dynamic_param.enable = SmrEn;               /* SMR module enabler */
            smr_dynamic_param.averaging_time = SmrAvgTime;
            error = AUDIO_PLAYER_Dynamic_Cfg();

            if (error != SMR_ERROR_NONE) {
                return (AUDIO_ERROR_INVALID_VALUE);
            }

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
            BSP_LCD_FillRect(0, LINE(3), TOUCH_SMR_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            sprintf((char *)str, "Playing file (%d/%d): %s",
                    file_idx + 1, FileList.ptr,
                    (char *)FileList.file[file_idx].name);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(4), TOUCH_SMR_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAtLine(4, str);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(5), TOUCH_SMR_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            sprintf((char *)str,  "Sample rate : %d Hz", (int)(info->SampleRate));
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(6), TOUCH_SMR_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAtLine(6, str);
            sprintf((char *)str,  "Channels number : %d", info->NbrChannels);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(7), TOUCH_SMR_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAtLine(7, str);
            duration = info->FileSize / info->ByteRate;
            sprintf((char *)str, "File Size : %d KB [%02d:%02d]", (int)(info->FileSize / 1024), (int)(duration / 60), (int)(duration % 60));
            BSP_LCD_ClearStringLine(8);
            BSP_LCD_DisplayStringAtLine(8, str);
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            BSP_LCD_DisplayStringAt(PLAYER_COUNT_TEXT_XMIN, LINE(PLAYER_COUNT_TEXT_LINE), (uint8_t *)"[00:00]", LEFT_MODE);
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
    /* ON/OFF module rectangle */
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_DrawRect(TOUCH_SMR_EN_XMIN, TOUCH_SMR_EN_YMIN , /* Smr rectangle */
                     TOUCH_SMR_EN_XMAX - TOUCH_SMR_EN_XMIN,
                     TOUCH_SMR_EN_YMAX - TOUCH_SMR_EN_YMIN);
    BSP_LCD_DisplayStringAt(TOUCH_SMR_EN_XMIN + TOUCH_SMR_EN_XOFFSET, LINE(4), (uint8_t *)"SMR ON/OFF", LEFT_MODE);
    /* Smr minus setting icon */
    BSP_LCD_FillRect(TOUCH_SMR_PREVIOUS_AVGTIME_XMIN, TOUCH_SMR_PREVIOUS_AVGTIME_YMIN + (TOUCH_SMR_PREVIOUS_AVGTIME_YMAX - TOUCH_SMR_PREVIOUS_AVGTIME_YMIN) / 2 - 5 , TOUCH_SMR_PREVIOUS_AVGTIME_YMAX - TOUCH_SMR_PREVIOUS_AVGTIME_YMIN, 10);
    /* Smr plus setting icon */
    BSP_LCD_FillRect(TOUCH_SMR_NEXT_AVGTIME_XMIN, TOUCH_SMR_NEXT_AVGTIME_YMIN + (TOUCH_SMR_PREVIOUS_AVGTIME_YMAX - TOUCH_SMR_PREVIOUS_AVGTIME_YMIN) / 2 - 5 , TOUCH_SMR_NEXT_AVGTIME_YMAX - TOUCH_SMR_NEXT_AVGTIME_YMIN, 10);
    BSP_LCD_FillRect(TOUCH_SMR_NEXT_AVGTIME_XMIN + (TOUCH_SMR_PREVIOUS_AVGTIME_XMAX - TOUCH_SMR_PREVIOUS_AVGTIME_XMIN) / 2 - 5, TOUCH_SMR_NEXT_AVGTIME_YMIN , 10, TOUCH_SMR_NEXT_AVGTIME_YMAX - TOUCH_SMR_NEXT_AVGTIME_YMIN); //vertical rect
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
    BSP_LCD_DisplayStringAt(TOUCH_VOL_MINUS_XMIN + TOUCH_VOL_MINUS_XOFFSET, LINE(TOUCH_VOL_LINE), (uint8_t *)"VOl-", LEFT_MODE);
    BSP_LCD_DrawRect(TOUCH_VOL_PLUS_XMIN, TOUCH_VOL_PLUS_YMIN , /* VOl+ rectangle */
                     TOUCH_VOL_PLUS_XMAX - TOUCH_VOL_PLUS_XMIN,
                     TOUCH_VOL_PLUS_YMAX - TOUCH_VOL_PLUS_YMIN);
    BSP_LCD_DisplayStringAt(TOUCH_VOL_PLUS_XMIN + TOUCH_VOL_MINUS_XOFFSET, LINE(TOUCH_VOL_LINE), (uint8_t *)"VOl+", LEFT_MODE);
    AUDIO_DisplaySmrStaticSettings();
    AUDIO_DisplaySmrSettings();
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
}

/**
    @brief  Test touch screen state and modify audio state machine according to that
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
            } else if ((TS_State.touchX[0] > TOUCH_SMR_EN_XMIN) && (TS_State.touchX[0] < TOUCH_SMR_EN_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_SMR_EN_YMIN) && (TS_State.touchY[0] < TOUCH_SMR_EN_YMAX)) {
                if (SmrEn > 0) {
                    AudioCfgChange = AUDIO_CFG_SMR_DIS;
                } else {
                    AudioCfgChange = AUDIO_CFG_SMR_EN;
                }
            } else if ((TS_State.touchX[0] > TOUCH_SMR_NEXT_AVGTIME_XMIN) && (TS_State.touchX[0] < TOUCH_SMR_NEXT_AVGTIME_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_SMR_NEXT_AVGTIME_YMIN) && (TS_State.touchY[0] < TOUCH_SMR_NEXT_AVGTIME_YMAX + 5)) {
                if (SmrEn > 0) {
                    AudioCfgChange = AUDIO_CFG_SMR_AVGTIME_NEXT;
                }
            } else if ((TS_State.touchX[0] > TOUCH_SMR_PREVIOUS_AVGTIME_XMIN) && (TS_State.touchX[0] < TOUCH_SMR_PREVIOUS_AVGTIME_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_SMR_PREVIOUS_AVGTIME_YMIN) && (TS_State.touchY[0] < TOUCH_SMR_PREVIOUS_AVGTIME_YMAX + 5)) {
                if (SmrEn > 0) {
                    AudioCfgChange = AUDIO_CFG_SMR_AVGTIME_PREVIOUS;
                }
            }
        }
    }
}


/**
    @brief  Manages Audio dynamic changes during playback.
    @param  None
    @retval Audio error
*/
static int32_t AUDIO_PLAYER_Dynamic_Cfg(void) {
    uint8_t str[13];
    int32_t error = SMR_ERROR_NONE;

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

        case AUDIO_CFG_SMR_EN:
            smr_getConfig(&smr_dynamic_param, pSmrPersistentMem);
            SmrEn = 1;
            smr_dynamic_param.enable = SmrEn;                             /* Enables the effect */
            smr_dynamic_param.averaging_time = SmrAvgTime;
            smr_dynamic_param.filter_type = SMR_PREFILTER_AWEIGHTING;
            error = smr_setConfig(&smr_dynamic_param, pSmrPersistentMem);

            if (error != SMR_ERROR_NONE) {
                return error;
            }

            AUDIO_DisplaySmrStaticSettings();
            AUDIO_DisplaySmrSettings();
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_SMR_DIS:
            smr_getConfig(&smr_dynamic_param, pSmrPersistentMem);
            SmrEn = 0;
            smr_dynamic_param.enable = SmrEn;            /* Disable the effect */
            error = smr_setConfig(&smr_dynamic_param, pSmrPersistentMem);

            if (error != SMR_ERROR_NONE) {
                return error;
            }

            BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
            BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
            BSP_LCD_ClearStringLine(MSG_MOD_LINE);
            BSP_LCD_DisplayStringAt(TOUCH_SMR_PREVIOUS_AVGTIME_XMIN, LINE(MSG_MOD_LINE), (uint8_t *)"SMR Manager OFF", LEFT_MODE);  /* LCD RK043FN48H: 480*272 pixels */
            BSP_LCD_DisplayStringAt(10, LINE(8), (uint8_t *)"                    ", RIGHT_MODE);
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_SMR_AVGTIME_NEXT:
            smr_getConfig(&smr_dynamic_param, pSmrPersistentMem);

            if (++SmrAvgTime > AUDIO_CFG_SMR_AVGTIME_MAX) {
                SmrAvgTime = AUDIO_CFG_SMR_AVGTIME_MAX;
            }

            smr_dynamic_param.averaging_time = SmrAvgTime;         /* User preset defined above */
            error = smr_setConfig(&smr_dynamic_param, pSmrPersistentMem);

            if (error != SMR_ERROR_NONE) {
                return error;
            }

            AUDIO_DisplaySmrSettings();
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_SMR_AVGTIME_PREVIOUS:
            smr_getConfig(&smr_dynamic_param, pSmrPersistentMem);

            if (--SmrAvgTime < AUDIO_CFG_SMR_AVGTIME_MIN) {
                SmrAvgTime = AUDIO_CFG_SMR_AVGTIME_MIN;
            }

            smr_dynamic_param.averaging_time = SmrAvgTime;         /* User preset defined above */
            error = smr_setConfig(&smr_dynamic_param, pSmrPersistentMem);

            if (error != SMR_ERROR_NONE) {
                return error;
            }

            AUDIO_DisplaySmrSettings();
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_NONE:
        default:
            break;
    }

    return error;
}

static void AUDIO_DisplaySmrStaticSettings(void) {
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
    BSP_LCD_DisplayStringAt(TOUCH_SMR_EN_XMIN + CHAR(8), LINE(8), (uint8_t *)"[A-Weighting]", LEFT_MODE);
}

static void AUDIO_DisplaySmrSettings(void) {
    uint8_t str[40];
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
    BSP_LCD_ClearStringLine(MSG_MOD_LINE);
    sprintf((char *)str, "Avg Time: %d ms", SmrAvgTime);
    BSP_LCD_DisplayStringAt(TOUCH_SMR_PREVIOUS_AVGTIME_XMIN - CHAR(2), LINE(MSG_MOD_LINE), (uint8_t *)str, LEFT_MODE);
}
