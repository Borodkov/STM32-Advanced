/**
  ******************************************************************************
  * @file    bam_glo.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    15-Oct-2017
  * @brief   This file provides BAM routines.
  *          For more details about this Library, please refer to document
  *          "UM1778 User manual". 
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
  
#ifndef BAM_GLO_H
#define BAM_GLO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_fw_glo.h"

/* Exported constants --------------------------------------------------------*/

/*      E R R O R   V A L U E S                                               */
#define BAM_ERROR_NONE                           0
#define BAM_UNSUPPORTED_INTERLEAVING_MODE       -1
#define BAM_UNSUPPORTED_NUMBER_OF_CHANNELS      -2
#define BAM_UNSUPPORTED_NUMBER_OF_BYTEPERSAMPLE -3
#define BAM_UNSUPPORTED_XOVER_FREQUENCY         -4
#define BAM_UNSUPPORTED_MODE                    -5
#define BAM_BAD_HW                              -6
#define BAM_UNSUPPORTED_FRAME_SIZE              -7   


#define BAM_OUTPUT_MODE_2_0                   1
#define BAM_OUTPUT_MODE_2_1_WITH_LFE_SPLIT    2
#define BAM_OUTPUT_MODE_2_1_WITHOUT_LFE_SPLIT 3


/* Exported types ------------------------------------------------------------*/

struct bam_dynamic_param {
    int32_t limiter_release_time;
    int16_t bass_vol;
    int16_t freq_xover;
    int16_t enable;
    int16_t limiter_enable;
};
typedef struct bam_dynamic_param bam_dynamic_param_t;

struct bam_static_param {
    int16_t mode;
};
typedef struct bam_static_param bam_static_param_t;


/* External variables --------------------------------------------------------*/

extern const uint32_t bam_scratch_mem_size;
extern const uint32_t bam_persistent_mem_size;


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* bam_reset() : initializes static memory, states machines, ... */
extern int32_t bam_reset(void *persistent_mem_ptr, void *scratch_mem_ptr);

/* bam_setConfig() : use to change dynamically some parameters */
extern int32_t bam_setConfig(bam_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);

/* bam_getConfig() : use to get values of dynamic parameters */
extern int32_t bam_getConfig(bam_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);

/* bam_setParam() : use to set parameters that won't change during processing */
extern int32_t bam_setParam(bam_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);

/* bam_getParam() : use to get values of static parameters */
extern int32_t bam_getParam(bam_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);

/* bam_process() : this is the main processing routine */
extern int32_t bam_process(buffer_t *input_buffer, buffer_t *output_buffer, void *persistent_mem_ptr);

#ifdef __cplusplus
}
#endif

#endif /* BAM_GLO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
