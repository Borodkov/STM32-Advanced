/**
  ******************************************************************************
  * @file    gam_glo.h
  * @author  MCD Application Team
  * @version V1.0.3
  * @date    15-Oct-2017
  * @brief   This file provides GAM routines.
  *          For more details about this Library, please refer to document
  *          "UM2006 User manual".
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2017 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Image SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_image_v2
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef GAM_GLO_H
#define GAM_GLO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_fw_glo.h"

/* Exported constants --------------------------------------------------------*/

#define GAM_VOL_MAX                    (0 << 1) /* Max gain to apply in dB above full scale in Q15.1 format => 0 dB*/
#define GAM_VOL_MIN                    (-80 << 1) /* Min gain to apply in dB below full scale in Q15.1 format => -60 dB*/

#define GAM_MONO                        1
#define GAM_STEREO                      2
#define GAM_NB_CHAN_MAX                 8

#define GAM_MUTE_NOT_ACTIVATED          0
#define GAM_MUTE_ACTIVATED             (GAM_MUTE_NOT_ACTIVATED + 1)

/*      E R R O R   V A L U E S                                               */

#define GAM_ERROR_NONE                              0
#define GAM_UNSUPPORTED_VOLUME                     -1
#define GAM_UNSUPPORTED_MUTE_MODE                  -2       
#define GAM_UNSUPPORTED_NUMBER_OF_BYTEPERSAMPLE    -3
#define GAM_UNSUPPORTED_INTERLEAVING               -4
#define GAM_UNSUPPORTED_MULTICHANNEL               -5
#define GAM_BAD_HW                                 -6

/* Exported types ------------------------------------------------------------*/

struct gam_dynamic_param {
    int16_t target_volume_dB[GAM_NB_CHAN_MAX];    /* in dB    */
    int16_t mute[GAM_NB_CHAN_MAX];                /* 0 = unmute, 1 = mute */
};
typedef struct gam_dynamic_param gam_dynamic_param_t;


struct gam_static_param {
     int8_t dummy;
};
typedef struct gam_static_param gam_static_param_t;

/* External variables --------------------------------------------------------*/

extern const uint32_t gam_scratch_mem_size;
extern const uint32_t gam_persistent_mem_size;


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* gam_reset() : initializes static memory, states machines, ... */
extern int32_t gam_reset(void *persistent_mem_ptr, void *scratch_mem_ptr);
/* gam_setConfig() : use to change dynamically some parameters */
extern int32_t gam_setConfig(gam_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);
/* gam_getConfig() : use to get values of dynamic parameters */
extern int32_t gam_getConfig(gam_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);
/* gam_setParam() : use to set parameters that won't change during processing */
extern int32_t gam_setParam(gam_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);
/* gam_getParam() : use to get values of static parameters */
extern int32_t gam_getParam(gam_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);
/* gam_process() : this is the main processing routine */
extern int32_t gam_process(buffer_t *input_buffer, buffer_t *output_buffer, void *persistent_mem_ptr);

#ifdef __cplusplus
}
#endif

#endif /* GAM_GLO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
