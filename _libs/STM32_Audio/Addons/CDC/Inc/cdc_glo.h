/**
  ******************************************************************************
  * @file    cdc_glo.h
  * @author  MCD Application Team
  * @version V1.0.2
  * @date    15-Oct-2017
  * @brief   This file provides CDC routines.
  *          For more details about this Library, please refer to document
  *          "UM1995 User manual". 
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
#ifndef CDC_GLO_H
#define CDC_GLO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_fw_glo.h"

/* Exported constants --------------------------------------------------------*/

/*      C O D E   C O N F I G U R A T I O N                                   */
/* Settings for Test/Main framework */
#define     DOWNSAMPLING_MODE           0
#define     UPSAMPLING_MODE             DOWNSAMPLING_MODE+1

/*      E R R O R   N U M B E R S                                             */
#define CDC_ERROR_NONE                          0
#define CDC_IOBUFFERS_TOO_SMALL                 -1
#define CDC_UNSUPPORTED_NUM_CHANNELS            -2
#define CDC_WRONG_NBBYTES_PER_SAMPLES           -3
#define CDC_INCONSISTENT_BUFFERSIZE_WITH_MODE   -4
#define CDC_UNSUPPORTED_MODE                    -5
#define CDC_UNSUPPORTED_INPLACE_PROCESSING      -6
#define CDC_BAD_HW                              -7

/* Exported types ------------------------------------------------------------*/

struct cdc_dynamic_param {
    uint32_t cdc_mode;
};
typedef struct cdc_dynamic_param cdc_dynamic_param_t;

struct cdc_static_param {
    int32_t   empty;        /* Need to have a non empty structure... */
};
typedef struct cdc_static_param cdc_static_param_t;

/* External variables --------------------------------------------------------*/

extern const uint32_t cdc_scratch_mem_size;
extern const uint32_t cdc_persistent_mem_size;


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

/* cdc_reset() : initializes static memory, states machines, ... */
extern int32_t cdc_reset(void *persistent_mem_ptr, void *scratch_mem_ptr);
/* cdc_setConfig() : use to change dynamically some parameters */
extern int32_t cdc_setConfig(cdc_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);
/* cdc_getConfig() : use to get values of dynamic parameters */
extern int32_t cdc_getConfig(cdc_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);
/* cdc_setParam() : use to set parameters that won't change during processing */
extern int32_t cdc_setParam(cdc_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);
/* cdc_getParam() : use to get values of static parameters */
extern int32_t cdc_getParam(cdc_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);
/* cdc_process() : this is the main processing routine */
extern int32_t cdc_process(buffer_t *input_buffer, buffer_t *output_buffer, void *persistent_mem_ptr);


#ifdef __cplusplus
}
#endif

#endif /* CDC_GLO_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
