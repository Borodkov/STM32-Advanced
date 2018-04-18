/**
  ******************************************************************************
  * @file    sdr_glo.h
  * @author  MCD Application Team
  * @version V1.0.4
  * @date    15-Oct-2017
  * @brief   Global header for SoundDetectoR module
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
#ifndef SDR_GLO_H
#define SDR_GLO_H

#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "audio_fw_glo.h"

/* Exported constants --------------------------------------------------------*/

/*      E R R O R   V A L U E S                                               */
#define SDR_ERROR_NONE                           0
#define SDR_UNSUPPORTED_INTERLEAVING_MODE       -1
#define SDR_UNSUPPORTED_NUMBER_OF_CHANNELS      -2
#define SDR_UNSUPPORTED_NUMBER_OF_BYTEPERSAMPLE -3
#define SDR_BAD_LEARNING_FRAME_NB               -4
#define SDR_BAD_THR_VALUE                       -5
#define SDR_BAD_NOISE_PWR_MIN                   -6
#define SDR_BAD_HW                              -7
#define SDR_LOOPSIZE_BUFFERSIZE_DIFFERENT       -8

/*      PRE-FILTER VALUES                                                     */
#define SDR_STATE_INACTIVE  0
#define SDR_STATE_ACTIVE    1

#define SDR_DISABLE                0
#define SDR_ENABLE_DUAL_HANGOVER   1
#define SDR_ENABLE_SINGLE_HANGOVER 2

/* Exported types ------------------------------------------------------------*/

struct sdr_dynamic_param {
    int32_t enable;               /* input variable  */
    int32_t ratio_thr1_dB;        /* input variable  */
    int32_t ratio_thr2_dB;        /* input variable  */
    int32_t noise_pwr_min_dB;     /* input variable  */
    int32_t hangover_nb_frame;    /* input variable  */
    int32_t output_state;         /* output variable */
    int32_t output_nrj;           /* output variable */
    int32_t output_noise_pwr;     /* output variable */
    int32_t output_thr1;          /* output variable */
    int32_t output_thr2;          /* output variable */
};
typedef struct sdr_dynamic_param sdr_dynamic_param_t;

struct sdr_static_param {
    int32_t  sampling_rate;
    int32_t  buffer_size;
    int32_t  learning_frame_nb;
};
typedef struct sdr_static_param sdr_static_param_t;


/* External variables --------------------------------------------------------*/

extern const uint32_t sdr_scratch_mem_size;
extern const uint32_t sdr_persistent_mem_size;


/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */ 

/* sdr_reset() : initializes static memory, states machines, ... */
extern int32_t sdr_reset(void *persistent_mem_ptr, void *scratch_mem_ptr);

/* sdr_setConfig() : use to change dynamically some parameters */
extern int32_t sdr_setConfig(sdr_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);

/* sdr_getConfig() : use to get values of dynamic parameters */
extern int32_t sdr_getConfig(sdr_dynamic_param_t *input_dynamic_param_ptr, void *persistent_mem_ptr);

/* sdr_getParam() : use to get values of static parameters */
extern int32_t sdr_getParam(sdr_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);

/* sdr_setParam() : use to set parameters that won't change during processing */
extern int32_t sdr_setParam(sdr_static_param_t *input_static_param_ptr, void *persistent_mem_ptr);

/* sdr_process() : this is the main processing routine */
extern int32_t sdr_process(buffer_t *input_buffer, buffer_t *output_buffer, void *persistent_mem_ptr);

#ifdef __cplusplus
}
#endif

#endif // SDR_GLO_H

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
