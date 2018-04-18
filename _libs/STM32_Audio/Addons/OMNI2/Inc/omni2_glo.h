/**
  ******************************************************************************
  * @file    omni2_glo.h
  * @author  MCD Application Team
  * @version V1.0.3
  * @date    15-Oct-2017
  * @brief   This file provides OMNI2 routines.
  *          For more details about this Library, please refer to documents
  *          "UM1633 User manual for Stereo Widening and UM1655 for MultiChannel".
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
#ifndef _OMNI2_GLO_H_
#define _OMNI2_GLO_H_

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_fw_glo.h"

/* Exported constants --------------------------------------------------------*/

enum OMNI2_AcMode_Supported_e
{
	AMODE20t = 0x0,          /*  Stereo channels for dolby pro logic */
  AMODE10 = 0x1,           /*  Mono channel (1.0) */
	AMODE20 = 0x2,           /*  Stereo channels (2.0) */
	AMODE30 = 0x3,           /*  Stereo + Center channel (3.0) */
  AMODE32 = 0x7,           /*  Stereo + Center channel + Surround Channels (5.0) */
  AMODE34 = 0xB,           /*  Stereo + Center channel + Surround Channels + Center Surround Channels (7.0) */
  AMODE20t_LFE = 0x80,     /*  Stereo channels for dolby pro logic + LFE channel */
  AMODE20_LFE = 0x82,      /*  Stereo + LFE channel (2.1) */
  AMODE30_LFE = 0x83,      /*  Stereo + Center channel + LFE channel (3.1) */
  AMODE32_LFE = 0x87,      /*  Stereo + Center channel + LFE channel + Surround Channels (5.1) */
  AMODE34_LFE = 0x8B,      /*  Stereo + Center channel + LFE channel + Surround Channels + Center Surround Channels (7.1) */
  AMODE_ID  = 0xFF         /*  End of supported configurations */
};


/*      C O D E   C O N F I G U R A T I O N                                   */

#define OMNI2_LISTENING_ANGLE_10            (10)
#define OMNI2_LISTENING_ANGLE_20            (20)
#define OMNI2_LISTENING_ANGLE_30            (30)


/*      E R R O R   V A L U E S                                               */

#define	OMNI2_ERROR_NONE            0
#define	OMNI2_ERROR                 -1
#define	OMNI2_ERROR_PARSE_COMMAND   -2
#define	OMNI2_BAD_HW                -3


/* Exported types ------------------------------------------------------------*/

struct omni2_dynamic_param {
    int32_t             Omni2Enable;
    int32_t             Omni2Strength;
    int32_t             Omni2ListeningAngle;
};
typedef struct omni2_dynamic_param omni2_dynamic_param_t;


struct omni2_static_param {
    int32_t             Omni2CentreOutput;
    int32_t             AudioMode;
    int32_t             SamplingFreq;
};
typedef struct omni2_static_param omni2_static_param_t;


/* External variables --------------------------------------------------------*/

extern const uint32_t omni2_scratch_mem_size;
extern const uint32_t omni2_persistent_mem_size;


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* omni2_reset() : initializes static memory, states machines, ... */
extern int32_t omni2_reset(void *persistent_mem_ptr, void *scratch_mem_ptr);
/* omni2_setConfig() : use to change dynamically some parameters */
extern int32_t omni2_setConfig(omni2_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);
/* omni2_getConfig() : use to get values of dynamic parameters */
extern int32_t omni2_getConfig(omni2_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);
/* omni2_setParam() : use to set parameters that won't change during processing */
extern int32_t omni2_setParam(omni2_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);
/* omni2_getParam() : use to get values of static parameters */
extern int32_t omni2_getParam(omni2_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);
/* omni2_process() : this is the main processing routine */
extern int32_t omni2_process(buffer_t *input_buffer, buffer_t *output_buffer, void *persistent_mem_ptr);

#ifdef __cplusplus
}
#endif

#endif /*_OMNI2_GLO_H_*/

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
