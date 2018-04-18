/**
  ******************************************************************************
    @file    Audio/Audio_BAM/Src/menu.c
    @author  MCD Application Team
    @brief   This file implements Menu Functions
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Global extern variables ---------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
AUDIO_PLAYBACK_StateTypeDef AudioState;

/* Private function prototypes -----------------------------------------------*/
static void AUDIO_ChangeSelectMode(AUDIO_DEMO_SelectMode select_mode);
static void LCD_ClearTextZone(void);

/* Private functions ---------------------------------------------------------*/

/**
    @brief  Manages AUDIO Menu Process.
    @param  None
    @retval None
*/
void AUDIO_MenuProcess(void) {
    if (appli_state == APPLICATION_READY) {
        if (AudioState == AUDIO_STATE_IDLE) {
            if (AUDIO_ShowWavFiles() > 0) {
                LCD_ErrLog("There is no WAV file on the USB Key.\n");
                AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
            } else {
                /* Start Playing */
                AudioState = AUDIO_STATE_INIT;
            }
        } else { /* Not idle */
            if (AudioState == AUDIO_STATE_START) {
                /* Clear the LCD */
                LCD_ClearTextZone();
            }

            /* Execute Audio Process state machine */
            if (AUDIO_PLAYER_Process() == AUDIO_ERROR_IO ||
                    AUDIO_PLAYER_Process() == AUDIO_ERROR ||
                    AUDIO_PLAYER_Process() == AUDIO_ERROR_INVALID_VALUE) {
                /* Clear the LCD */
                LCD_ClearTextZone();
                AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
            }
        }
    }

    if (appli_state == APPLICATION_DISCONNECT) {
        appli_state = APPLICATION_IDLE;
        AUDIO_ChangeSelectMode(AUDIO_SELECT_MENU);
        BSP_AUDIO_OUT_Stop(CODEC_PDWN_SW);
    }
}

/*******************************************************************************
                            Static Functions
*******************************************************************************/
/**
    @brief  Changes the selection mode.
    @param  select_mode: Selection mode
    @retval None
*/
static void AUDIO_ChangeSelectMode(AUDIO_DEMO_SelectMode select_mode) {
    if (select_mode == AUDIO_SELECT_MENU) {
        LCD_LOG_UpdateDisplay();
    } else if (select_mode == AUDIO_PLAYBACK_CONTROL) {
        LCD_ClearTextZone();
    }
}

/**
    @brief  Clears the text zone.
    @param  None
    @retval None
*/
static void LCD_ClearTextZone(void) {
    uint8_t i = 0;

    for (i = 0; i < 13; i++) {
        BSP_LCD_ClearStringLine(i + 3);
    }
}
