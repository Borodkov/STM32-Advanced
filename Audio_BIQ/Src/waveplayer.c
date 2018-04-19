/**
  ******************************************************************************
    @file    Audio/Audio_BIQ/Src/waveplayer.c
    @author  MCD Application Team
    @brief   This file provides the Audio Out (playback) interface API
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "audio_fw_glo.h"
#include "biq_glo.h"

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

#define TOUCH_BIQ_EN_XMIN      338
#define TOUCH_BIQ_EN_XMAX      456
#define TOUCH_BIQ_EN_YMIN      55
#define TOUCH_BIQ_EN_YMAX      85
#define TOUCH_BIQ_EN_XOFFSET   5
#define TOUCH_BIQ_PARAM_LINE   8

#define TOUCH_BIQ_NEXT_XMIN    412
#define TOUCH_BIQ_NEXT_XMAX    452
#define TOUCH_BIQ_NEXT_YMIN    130
#define TOUCH_BIQ_NEXT_YMAX    170

#define TOUCH_BIQ_PREVIOUS_XMIN    342
#define TOUCH_BIQ_PREVIOUS_XMAX    382
#define TOUCH_BIQ_PREVIOUS_YMIN    130
#define TOUCH_BIQ_PREVIOUS_YMAX    170

#define PLAYER_COUNT_TEXT_XMIN  263
#define PLAYER_COUNT_TEXT_LINE   8
#define MSG_MOD_LINE            15

#define CHAR(x) ((x) * (((sFONT *)BSP_LCD_GetFont())->Width))
/* Private macro -------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static AUDIO_OUT_BufferTypeDef  BufferCtl;
static int16_t FilePos = 0;
static int16_t BiqEn = 1;
static __IO uint32_t uwVolume = 50;

/*-------------------------------*/
static int16_t FilterSelectType = BIQ_FILT_PEAK;
static int16_t NBBIQSec = NB_BIQUAD_SEC_2;

/*content: nb_sos, post-shift, b0, b1, a0, a1, a2*/
static int32_t *tab_coeff;

/*-------------------------------*/
/*-- These coefficients are computed for a sampling rate = 48kHz --*/

/*PEAK_2000hz_order4_48k.txt*/
static const int32_t tab_coeff_Peak[1 + NB_BIQUAD_SEC_2 * 6] = { 2, 1, 1112413609, -1978318489, 935692401, 1978318489, -974364186, \
                                                                 2, 474885868, -941646307, 474885868, 941646307, -412900824
                                                               };
/*LOUDNESS.txt*/
//static const int32_t tab_coeff_Loudness[1+NB_BIQUAD_SEC_1*6]={ 1,0,1201999107,424590732,3068307,344502132,-1644547};
static const int32_t tab_coeff_Loudness[1 + NB_BIQUAD_SEC_5 * 6] = {5, 0, 1201999107, 424590732, 3068307, 344502132, -1644547, \
                                                                    1, 1073741824, -2075485822, 1010128930, 1905720500, -845892267, \
                                                                    1, 1073741824, -1348856775, 812477686, 1121328227, -685022681, \
                                                                    1, 1073741824, -1791523805, 845106364, 1415657206, -407278556, \
                                                                    2, 1134670788, 0, 0, 0, 0
                                                                   };

/*NOTCH_1000Hz_order4_48kHz.txt*/
static const int32_t tab_coeff_Notch[1 + NB_BIQUAD_SEC_2 * 6] = { 2, 1, 949771736, -1883292613, 949771736, 1883292613, -825801648, \
                                                                  2, 474885868, -941646307, 474885868, 941646307, -412900824
                                                                };
/*voiceEnhancer_9Biq_48kHz.txt*/
static const int32_t tab_coeff_Voice[1 + NB_BIQUAD_SEC_9 * 6] = { 9, 1, 1245540480, -174375680, 1619200, 607744, -17152, \
                                                                  1, 1197222144, -1111143168, 705231232, 1104001280, -821426048, \
                                                                  1, 1225676288, -1365175552, 616297984, 1336853248, -742198272, \
                                                                  1, 1165009920, -1609803520, 690196224, 1601686784, -774282368, \
                                                                  1, 1086089856, -1932872192, 994070784, 1929527808, -1003136896, \
                                                                  1, 1155346176, -1978276608, 884561920, 1969965312, -958188416, \
                                                                  1, 1102926080, -2038896128, 962226688, 2039336192, -991850112, \
                                                                  1, 1062682240, -2048568832, 998847232, 2049392128, -988612352, \
                                                                  1, 756638378, -1513276937, 756638468, 2132658432, -1059033600
                                                                };

/* Low-pass, fc=150hz, 3 times same SOS */
const int32_t tab_coeff_Lowpass[1 + NB_BIQUAD_SEC_3 * 6] = {3, 1, 102069, 204139, 102069, 2117665461, -1044331918, \
                                                            1, 102069, 204139, 102069, 2117665461, -1044331918, \
                                                            1, 102069, 204139, 102069, 2117665461, -1044331918
                                                           };

/* High-pass, fc=150hz, 3 times same SOS */
const int32_t tab_coeff_Highpass[1 + NB_BIQUAD_SEC_3 * 6] = {3, 1, 1058934800, -2117869603, 1058934800, 2117665461, -1044331918, \
                                                             1, 1058934800, -2117869603, 1058934800, 2117665461, -1044331918, \
                                                             1, 1058934800, -2117869603, 1058934800, 2117665461, -1044331918
                                                            };


/*bypass.txt*/
static const int32_t tab_coeff_Bypass[1 + 6] = {1, 0, 2147483647, 0, 0, 0, 0};

/*-------------------------------*/

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

/* BiQuad filter*/
void *pBIQPersistentMem = NULL;
void *pBIQScratchMem = NULL;

static  biquad_df1_cascade_static_param_t  biquad_df1_cascade_static_param;
static  biquad_df1_cascade_dynamic_param_t biquad_df1_cascade_dynamic_param;

static  buffer_t BufferHandler;
static  buffer_t *pBufferHandler = &BufferHandler;

/* Private function prototypes -----------------------------------------------*/
static AUDIO_ErrorTypeDef GetFileInfo(uint16_t file_idx, WAVE_FormatTypeDef *info);
static uint8_t PlayerInit(uint32_t AudioFreq);
static void AUDIO_PlaybackDisplayButtons(void);
static void AUDIO_AcquireTouchButtons(void);
static int32_t  AUDIO_PLAYER_Dynamic_Cfg(void);

static void AUDIO_DisplayBiqSettings(void);
static void AUDIO_FilterConfig(biquad_df1_cascade_dynamic_param_t *biquad_df1_cascade_dyn, int16_t FilterSelectType);

/* Private functions ---------------------------------------------------------*/

/**
    @brief  Initializes Audio Interface.
    @param  None
    @retval Audio error
*/
AUDIO_ErrorTypeDef AUDIO_PLAYER_Init(void) {
    /* Allocat mem for Biq */
    pBIQPersistentMem = malloc(biquad_df1_cascade_persistent_mem_size); /* biquad_df1_cascade_persistent_mem_size  0x025C */
    pBIQScratchMem = malloc(biquad_df1_cascade_scratch_mem_size);       /* biquad_df1_cascade_scratch_mem_size  0x04 */

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
    int32_t error = BIQ_ERROR_NONE;
    int32_t k = 0;
    f_close(&WavFile);

    if (AUDIO_GetWavObjectNumber() > idx) {
        BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
        BSP_LCD_FillRect(0, LINE(3), BSP_LCD_GetXSize() , LINE(1));
        GetFileInfo(idx, &WaveFormat);

        if (WaveFormat.SampleRate != 48000) {
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(TOUCH_BIQ_EN_XMIN, TOUCH_BIQ_EN_YMIN, TOUCH_BIQ_EN_XMAX - TOUCH_BIQ_EN_XMIN + 2, TOUCH_BIQ_EN_YMAX - TOUCH_BIQ_EN_YMIN + 2);
            BSP_LCD_SetTextColor(LCD_COLOR_RED);
            BSP_LCD_DisplayStringAtLine(MSG_MOD_LINE, (uint8_t *) "Error: This sample frequency is not supported. Try another track.");
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            f_close(&WavFile);
            AudioState = AUDIO_STATE_STOP;
            return (AUDIO_ERROR_INVALID_VALUE);
        }

        if (WaveFormat.NbrChannels != 2) {
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(TOUCH_BIQ_EN_XMIN, TOUCH_BIQ_EN_YMIN, TOUCH_BIQ_EN_XMAX - TOUCH_BIQ_EN_XMIN + 2, TOUCH_BIQ_EN_YMAX - TOUCH_BIQ_EN_YMIN + 2);
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
        /* BIQ effect reset */
        error = biquad_df1_cascade_reset(pBIQPersistentMem, pBIQScratchMem);

        if (error != BIQ_ERROR_NONE) {
            return (AUDIO_ERROR_SW);
        }

        /* BIQ effect static parameters setting */
        biquad_df1_cascade_static_param.nb_sos = tab_coeff_Peak[0]; /* by default set to:NB_BIQUAD_SEC_2 [2-second order sections] */
        tab_coeff = (int32_t *)&tab_coeff_Peak[1];

        for (k = 0 ; k < biquad_df1_cascade_static_param.nb_sos ; k++) {
            biquad_df1_cascade_static_param.biquad_coeff[k * 6]   = *tab_coeff++;
            biquad_df1_cascade_static_param.biquad_coeff[k * 6 + 1] = *tab_coeff++;
            biquad_df1_cascade_static_param.biquad_coeff[k * 6 + 2] = *tab_coeff++;
            biquad_df1_cascade_static_param.biquad_coeff[k * 6 + 3] = *tab_coeff++;
            biquad_df1_cascade_static_param.biquad_coeff[k * 6 + 4] = *tab_coeff++;
            biquad_df1_cascade_static_param.biquad_coeff[k * 6 + 5] = *tab_coeff++;
        }

        error = biquad_df1_cascade_setParam(&biquad_df1_cascade_static_param, pBIQPersistentMem);

        if (error != BIQ_ERROR_NONE) {
            return (AUDIO_ERROR_SW);
        }

        /* BIQ dynamic parameters that can be updated here every frame if required */
        biquad_df1_cascade_dynamic_param.enable = 1;                /* Enables the effect */
        biquad_df1_cascade_dynamic_param.nb_sos = tab_coeff_Peak[0]; /* by default set to:NB_BIQUAD_SEC_2 */
        tab_coeff = (int32_t *)&tab_coeff_Peak[1];

        for (k = 0 ; k < biquad_df1_cascade_dynamic_param.nb_sos ; k++) {
            biquad_df1_cascade_dynamic_param.biquad_coeff[k * 6]   = *tab_coeff++;
            biquad_df1_cascade_dynamic_param.biquad_coeff[k * 6 + 1] = *tab_coeff++;
            biquad_df1_cascade_dynamic_param.biquad_coeff[k * 6 + 2] = *tab_coeff++;
            biquad_df1_cascade_dynamic_param.biquad_coeff[k * 6 + 3] = *tab_coeff++;
            biquad_df1_cascade_dynamic_param.biquad_coeff[k * 6 + 4] = *tab_coeff++;
            biquad_df1_cascade_dynamic_param.biquad_coeff[k * 6 + 5] = *tab_coeff++;
        }

        error = biquad_df1_cascade_setConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);

        if (error != BIQ_ERROR_NONE) {
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
    int32_t error = BIQ_ERROR_NONE;

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
                error = biquad_df1_cascade_process(pBufferHandler, pBufferHandler, pBIQPersistentMem);

                if (error != BIQ_ERROR_NONE) {
                    return (AUDIO_ERROR_SW);
                }

                /* Dynamic change done when switching buffer */
                /* Related dispay processing takes sometime hence better to do it just after a buffer is refilled */
                error = AUDIO_PLAYER_Dynamic_Cfg();

                if (error != BIQ_ERROR_NONE) {
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
                error = biquad_df1_cascade_process(pBufferHandler, pBufferHandler, pBIQPersistentMem);

                if (error != BIQ_ERROR_NONE) {
                    return (AUDIO_ERROR_SW);
                }

                /* Dynamic change done when switching buffer */
                /* Related dispay processing takes sometime hence better to do it just after a buffer is refilled */
                error = AUDIO_PLAYER_Dynamic_Cfg();

                if (error != BIQ_ERROR_NONE) {
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

            /* BIQ dynamic parameters that can be updated here every frame if required */
            biquad_df1_cascade_getConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);
            biquad_df1_cascade_dynamic_param.enable = BiqEn;
            AUDIO_FilterConfig(&biquad_df1_cascade_dynamic_param, FilterSelectType);
            error = biquad_df1_cascade_setConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);

            if (error != BIQ_ERROR_NONE) {
                return (AUDIO_ERROR_SW);
            }

            AUDIO_DisplayBiqSettings();
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
            AUDIO_FilterConfig(&biquad_df1_cascade_dynamic_param, FilterSelectType);
            error = AUDIO_PLAYER_Dynamic_Cfg();

            if (error != BIQ_ERROR_NONE) {
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
            BSP_LCD_FillRect(0, LINE(3), TOUCH_BIQ_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            sprintf((char *)str, "Playing file (%d/%d): %s",
                    file_idx + 1, FileList.ptr,
                    (char *)FileList.file[file_idx].name);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(4), TOUCH_BIQ_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAtLine(4, str);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(5), TOUCH_BIQ_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
            sprintf((char *)str,  "Sample rate : %d Hz", (int)(info->SampleRate));
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(6), TOUCH_BIQ_EN_XMIN , LINE(1));
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAtLine(6, str);
            sprintf((char *)str,  "Channels number : %d", info->NbrChannels);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(0, LINE(7), TOUCH_BIQ_EN_XMIN , LINE(1));
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
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_DrawRect(TOUCH_BIQ_EN_XMIN, TOUCH_BIQ_EN_YMIN , /* Biq rectangle */
                     TOUCH_BIQ_EN_XMAX - TOUCH_BIQ_EN_XMIN,
                     TOUCH_BIQ_EN_YMAX - TOUCH_BIQ_EN_YMIN);
    BSP_LCD_DisplayStringAt(TOUCH_BIQ_EN_XMIN + TOUCH_BIQ_EN_XOFFSET, LINE(4), (uint8_t *)"BIQ ON/OFF", LEFT_MODE);
    /* minus setting icon */
    BSP_LCD_FillRect(TOUCH_BIQ_PREVIOUS_XMIN, TOUCH_BIQ_PREVIOUS_YMIN + (TOUCH_BIQ_PREVIOUS_YMAX - TOUCH_BIQ_PREVIOUS_YMIN) / 2 - 5 , TOUCH_BIQ_PREVIOUS_YMAX - TOUCH_BIQ_PREVIOUS_YMIN, 10);
    /* plus setting icon */
    BSP_LCD_FillRect(TOUCH_BIQ_NEXT_XMIN, TOUCH_BIQ_NEXT_YMIN + (TOUCH_BIQ_PREVIOUS_YMAX - TOUCH_BIQ_PREVIOUS_YMIN) / 2 - 5 , TOUCH_BIQ_NEXT_YMAX - TOUCH_BIQ_NEXT_YMIN, 10);
    BSP_LCD_FillRect(TOUCH_BIQ_NEXT_XMIN + (TOUCH_BIQ_PREVIOUS_XMAX - TOUCH_BIQ_PREVIOUS_XMIN) / 2 - 5, TOUCH_BIQ_NEXT_YMIN , 10, TOUCH_BIQ_NEXT_YMAX - TOUCH_BIQ_NEXT_YMIN); /* vertical */
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
    AUDIO_DisplayBiqSettings();
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
            } else if ((TS_State.touchX[0] > TOUCH_BIQ_EN_XMIN) && (TS_State.touchX[0] < TOUCH_BIQ_EN_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_BIQ_EN_YMIN) && (TS_State.touchY[0] < TOUCH_BIQ_EN_YMAX)) {
                if (BiqEn > 0) {
                    AudioCfgChange = AUDIO_CFG_BIQ_DIS;
                } else {
                    AudioCfgChange = AUDIO_CFG_BIQ_EN;
                }
            } else if ((TS_State.touchX[0] > TOUCH_BIQ_NEXT_XMIN) && (TS_State.touchX[0] < TOUCH_BIQ_NEXT_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_BIQ_NEXT_YMIN) && (TS_State.touchY[0] < TOUCH_BIQ_NEXT_YMAX)) {
                if (BiqEn > 0) {
                    AudioCfgChange = AUDIO_CFG_BIQ_NEXT;
                }
            } else if ((TS_State.touchX[0] > TOUCH_BIQ_PREVIOUS_XMIN) && (TS_State.touchX[0] < TOUCH_BIQ_PREVIOUS_XMAX) &&
                       (TS_State.touchY[0] > TOUCH_BIQ_PREVIOUS_YMIN) && (TS_State.touchY[0] < TOUCH_BIQ_PREVIOUS_YMAX)) {
                if (BiqEn > 0) {
                    AudioCfgChange = AUDIO_CFG_BIQ_PREVIOUS;
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
    int32_t error = BIQ_ERROR_NONE;

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

        case AUDIO_CFG_BIQ_EN:
            BiqEn = 1;
            biquad_df1_cascade_dynamic_param.enable = BiqEn;            /* Enables the effect */
            error = biquad_df1_cascade_setConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);

            if (error != BIQ_ERROR_NONE) {
                return error;
            }

            AUDIO_DisplayBiqSettings();
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_BIQ_DIS:
            BiqEn = 0;
            biquad_df1_cascade_dynamic_param.enable = BiqEn;            /* Disable the effect */
            error = biquad_df1_cascade_setConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);

            if (error != BIQ_ERROR_NONE) {
                return error;
            }

            BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
            BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
            BSP_LCD_ClearStringLine(MSG_MOD_LINE);
            BSP_LCD_DisplayStringAt(TOUCH_BIQ_EN_XMIN, LINE(MSG_MOD_LINE), (uint8_t *)"Biquad Filter OFF", LEFT_MODE);   /* LCD RK043FN48H: 480*272 pixels */
            BSP_LCD_DisplayStringAt(TOUCH_BIQ_EN_XMIN, LINE(TOUCH_BIQ_PARAM_LINE), (uint8_t *) "                     ", LEFT_MODE);
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_BIQ_NEXT:
            biquad_df1_cascade_getConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);

            if (++FilterSelectType > BIQ_FILT_MAX) {
                FilterSelectType = BIQ_FILT_MAX;
            }

            AUDIO_FilterConfig(&biquad_df1_cascade_dynamic_param, FilterSelectType);
            error = biquad_df1_cascade_setConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);

            if (error != BIQ_ERROR_NONE) {
                return error;
            }

            AUDIO_DisplayBiqSettings();
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_BIQ_PREVIOUS:
            biquad_df1_cascade_getConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);

            if (--FilterSelectType < BIQ_FILT_MIN) {
                FilterSelectType = BIQ_FILT_MIN;
            }

            AUDIO_FilterConfig(&biquad_df1_cascade_dynamic_param, FilterSelectType);
            error = biquad_df1_cascade_setConfig(&biquad_df1_cascade_dynamic_param, pBIQPersistentMem);

            if (error != BIQ_ERROR_NONE) {
                return error;
            }

            AUDIO_DisplayBiqSettings();
            AudioCfgChange = AUDIO_CFG_NONE;
            break;

        case AUDIO_CFG_NONE:
        default:
            break;
    }

    return error;
}

static void AUDIO_DisplayBiqSettings(void) {
    uint8_t str[45];
    BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
    BSP_LCD_SetFont(&LCD_LOG_TEXT_FONT);
    BSP_LCD_ClearStringLine(MSG_MOD_LINE);

    switch (FilterSelectType) {
        case BIQ_FILT_PEAK:
            sprintf((char *)str, "[NB_SOS = %d] BIQ Filter: PEAK", NBBIQSec);
            break;

        case BIQ_FILT_LOUDNESS:
            sprintf((char *)str, "[NB_SOS = %d] BIQ Filter: LOUDNESS", NBBIQSec);
            break;

        case BIQ_FILT_NOTCH:
            sprintf((char *)str, "[NB_SOS = %d] BIQ Filter: NOTCH", NBBIQSec);
            break;

        case BIQ_FILT_VOICE:
            sprintf((char *)str, "[NB_SOS = %d] BIQ Filter: VOICE", NBBIQSec);
            break;

        case BIQ_FILT_LPASS:
            sprintf((char *)str, "[NB_SOS = %d] BIQ Filter: LOW-PASS", NBBIQSec);
            break;

        case BIQ_FILT_HPASS:
            sprintf((char *)str, "[NB_SOS = %d] BIQ Filter: HIGH-PASS", NBBIQSec);
            break;

        case BIQ_FILT_BYPASS:
            sprintf((char *)str, "[NB_SOS = %d] BIQ Filter: BYPASS", NBBIQSec);
            break;
    }

    BSP_LCD_DisplayStringAt(TOUCH_BIQ_EN_XMIN - CHAR(15), LINE(MSG_MOD_LINE), (uint8_t *)str, LEFT_MODE);
}

static void AUDIO_FilterConfig(biquad_df1_cascade_dynamic_param_t *biquad_df1_cascade_dyn, int16_t FilterSelectType) {
    uint8_t k = 0;

    switch (FilterSelectType) {
        case BIQ_FILT_PEAK:
            NBBIQSec = tab_coeff_Peak[0];
            tab_coeff = (int32_t *)&tab_coeff_Peak[1];
            break;

        case BIQ_FILT_LOUDNESS:
            NBBIQSec = tab_coeff_Loudness[0];
            tab_coeff = (int32_t *)&tab_coeff_Loudness[1];
            break;

        case BIQ_FILT_NOTCH:
            NBBIQSec = tab_coeff_Notch[0];
            tab_coeff = (int32_t *)&tab_coeff_Notch[1];
            break;

        case BIQ_FILT_VOICE:
            NBBIQSec = tab_coeff_Voice[0];
            tab_coeff = (int32_t *)&tab_coeff_Voice[1];
            break;

        case BIQ_FILT_LPASS:
            NBBIQSec = tab_coeff_Lowpass[0];
            tab_coeff = (int32_t *)&tab_coeff_Lowpass[1];
            break;

        case BIQ_FILT_HPASS:
            NBBIQSec = tab_coeff_Highpass[0];
            tab_coeff = (int32_t *)&tab_coeff_Highpass[1];
            break;

        case BIQ_FILT_BYPASS:
            NBBIQSec = tab_coeff_Bypass[0];
            tab_coeff = (int32_t *)&tab_coeff_Bypass[1];
            break;
    }

    /* Update some dynamic parameters values*/
    biquad_df1_cascade_dyn->enable = BiqEn;
    biquad_df1_cascade_dyn->nb_sos = NBBIQSec;

    for (k = 0 ; k < biquad_df1_cascade_dyn->nb_sos ; k++) {
        biquad_df1_cascade_dyn->biquad_coeff[k * 6]   = *tab_coeff++; /*post-chift*/
        biquad_df1_cascade_dyn->biquad_coeff[k * 6 + 1] = *tab_coeff++; /*b0*/
        biquad_df1_cascade_dyn->biquad_coeff[k * 6 + 2] = *tab_coeff++; /*b1*/
        biquad_df1_cascade_dyn->biquad_coeff[k * 6 + 3] = *tab_coeff++; /*a0*/
        biquad_df1_cascade_dyn->biquad_coeff[k * 6 + 4] = *tab_coeff++; /*a1*/
        biquad_df1_cascade_dyn->biquad_coeff[k * 6 + 5] = *tab_coeff++; /*a2*/
    }
}
