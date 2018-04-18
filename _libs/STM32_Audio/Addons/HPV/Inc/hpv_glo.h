/**
  ******************************************************************************
  * @file    hpv_glo.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    15-Oct-2017
  * @brief   This file contains HeadPhone Virtualizer module definitions to export 
  *          For more details about this Library, please refer to document
  *          "UM2024 User manual".
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
#ifndef _HPV_GLO_H_
#define _HPV_GLO_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_fw_glo.h"

/* Exported constants --------------------------------------------------------*/

/*      D E F I N I T I O N S                                                 */
enum eHvpAcInput_Supported
{
  AINPUT_10 = 1,  /* C */
  AINPUT_20 = 2,  /* L, R */
  AINPUT_32 = 7,  /* L, R, C, Ls, Rs */
  AINPUT_34 = 11,  /* L, R, C, Ls, Rs, Csl, Csr */
  ANB_INPUT
};

/*      E R R O R   V A L U E S                                               */
#define HPV_ERROR_NONE                      (0)
#define HPV_BAD_HW                          (-1)
#define HPV_ERROR_BAD_STRENGTH              (-2)
#define HPV_ERROR_BAD_HEADTRACKING_AZIMUTH  (-3)
#define HPV_ERROR_BAD_AUDIOMODE             (-4)
#define HPV_ERROR_BAD_BUFFER_LENGTH         (-5)

/* Exported types ------------------------------------------------------------*/

struct hpv_dynamic_param {
    int32_t    HpvEnable;           /* 1 to enable HPV effect, else 0 */
    int32_t    HpvStrength;         /* 100 for max effect, 0 for min effect */
    int32_t    HpvHTEnable;         /* 1 to enable Head Tracking, else 0 */
    int32_t    HpvHTAzimuth;        /* Head rotation, value must be between -180 and +180 : +90 means head turned right 90 degrees */
};
typedef struct hpv_dynamic_param hpv_dynamic_param_t;


struct hpv_static_param {
    int32_t     HpvAudioMode;       /* from values enum eHvpAcInput_Supported */
    int32_t     HpvSamplingRate;    /* I/O Sampling rate in Hz */
    int32_t     HpvLfeEnable;       /* 1 if LFE in input stream (x.1 input), else 0 */
};
typedef struct hpv_static_param hpv_static_param_t;


/* External variables --------------------------------------------------------*/

extern const uint32_t hpv_scratch_mem_size;
extern const uint32_t hpv_persistent_mem_size;



/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* hpv_reset() : initializes static memory, states machines, ... */
extern int32_t hpv_reset(void *persistent_mem_ptr, void *scratch_mem_ptr);
/* hpv_setConfig() : use to change dynamically some parameters */
extern int32_t hpv_setConfig(hpv_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);
/* hpv_getConfig() : use to get values of dynamic parameters */
extern int32_t hpv_getConfig(hpv_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);
/* hpv_setParam() : use to set parameters that won't change during processing */
extern int32_t hpv_setParam(hpv_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);
/* hpv_getParam() : use to get values of static parameters */
extern int32_t hpv_getParam(hpv_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);
/* hpv_process() : this is the main processing routine */
extern int32_t hpv_process(buffer_t *input_buffer, buffer_t *output_buffer, void *persistent_mem_ptr);

#ifdef __cplusplus
}
#endif

#endif /*_HPV_GLO_H_*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
