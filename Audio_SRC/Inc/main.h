/**
  ******************************************************************************
    @file    Audio/Audio_SRC/Inc/main.h
    @author  MCD Application Team
    @brief   Header for main.c module
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include "stm32f7xx_hal.h"
#include "stm32746g_discovery.h"
#include "stm32746g_discovery_audio.h"
#include "stm32746g_discovery_ts.h"
#include "lcd_log.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "usbh_core.h"
#include "usbh_diskio.h"
#include "usbh_msc.h"

/* Exported Defines ----------------------------------------------------------*/
#define AUDIO_OUT_BUFFER_SIZE                      3840 /* 480 * 2 (half + half) * 2 (stereo) * 2 (bytes x sample) */

#define FILEMGR_LIST_DEPDTH                        24
#define FILEMGR_FILE_NAME_SIZE                     40
#define FILEMGR_FULL_PATH_SIZE                     256
#define FILEMGR_MAX_LEVEL                          4
#define FILETYPE_DIR                               0
#define FILETYPE_FILE                              1

/* Exported types ------------------------------------------------------------*/
/* Application State Machine Structure */
typedef enum {
    APPLICATION_IDLE = 0,
    APPLICATION_START,
    APPLICATION_READY,
    APPLICATION_DISCONNECT,
} AUDIO_ApplicationTypeDef;


typedef enum {
    AUDIO_STATE_IDLE = 0,
    AUDIO_STATE_WAIT,
    AUDIO_STATE_INIT,
    AUDIO_STATE_START,
    AUDIO_STATE_STOP,
    AUDIO_STATE_PLAY,
    AUDIO_STATE_NEXT,
    AUDIO_STATE_PREVIOUS,
    AUDIO_STATE_FORWARD,
    AUDIO_STATE_BACKWARD,
    AUDIO_STATE_PAUSE,
    AUDIO_STATE_RESUME,
    AUDIO_STATE_ERROR,
} AUDIO_PLAYBACK_StateTypeDef;

typedef enum {
    AUDIO_CFG_NONE,
    AUDIO_CFG_VOLUME_UP,
    AUDIO_CFG_VOLUME_DOWN,
    AUDIO_CFG_ERROR,
} AUDIO_PLAYBACK_CfgTypeDef;

typedef enum {
    AUDIO_SELECT_MENU = 0,
    AUDIO_PLAYBACK_CONTROL,
} AUDIO_DEMO_SelectMode;

typedef enum {
    BUFFER_OFFSET_NONE = 0,
    BUFFER_OFFSET_HALF,
    BUFFER_OFFSET_FULL,
} BUFFER_StateTypeDef;

/* Audio buffer control struct */
typedef struct {
    uint8_t buff[AUDIO_OUT_BUFFER_SIZE];
    BUFFER_StateTypeDef state;
    uint32_t fptr;
} AUDIO_OUT_BufferTypeDef;

typedef enum {
    BUFFER_EMPTY = 0,
    BUFFER_FULL,
} WR_BUFFER_StateTypeDef;


typedef struct {
    uint32_t ChunkID;       /* 0 */
    uint32_t FileSize;      /* 4 */
    uint32_t FileFormat;    /* 8 */
    uint32_t SubChunk1ID;   /* 12 */
    uint32_t SubChunk1Size; /* 16*/
    uint16_t AudioFormat;   /* 20 */
    uint16_t NbrChannels;   /* 22 */
    uint32_t SampleRate;    /* 24 */

    uint32_t ByteRate;      /* 28 */
    uint16_t BlockAlign;    /* 32 */
    uint16_t BitPerSample;  /* 34 */
    uint32_t SubChunk2ID;   /* 36 */
    uint32_t SubChunk2Size; /* 40 */
} WAVE_FormatTypeDef;

typedef struct _FILELIST_LineTypeDef {
    uint8_t type;
    uint8_t name[FILEMGR_FILE_NAME_SIZE];
} FILELIST_LineTypeDef;

typedef struct _FILELIST_FileTypeDef {
    FILELIST_LineTypeDef  file[FILEMGR_LIST_DEPDTH] ;
    uint16_t              ptr;
} FILELIST_FileTypeDef;

typedef enum {
    AUDIO_ERROR_NONE = 0,
    AUDIO_ERROR_IO,
    AUDIO_ERROR_EOF,
    AUDIO_ERROR_INVALID_VALUE,
    AUDIO_ERROR_CODEC,
    AUDIO_ERROR_SW,
} AUDIO_ErrorTypeDef;

extern USBH_HandleTypeDef hUSBHost;
extern AUDIO_ApplicationTypeDef appli_state;
extern AUDIO_PLAYBACK_StateTypeDef AudioState;
extern FATFS USBH_fatfs;
extern FIL WavFile;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
/* Menu API */
void AUDIO_MenuProcess(void);
uint8_t AUDIO_ShowWavFiles(void);

/* Disk Explorer API */
uint8_t AUDIO_StorageInit(void);
FRESULT AUDIO_StorageParse(void);
uint16_t AUDIO_GetWavObjectNumber(void);

#endif /* __MAIN_H */
