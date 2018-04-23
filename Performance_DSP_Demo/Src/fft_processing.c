/**
  ******************************************************************************
    @file    STM32F746_DSPDEMO\Src\fft_processing.c
    @author  MCD Application Team
    @brief   FFT calculation Service Routines
  ******************************************************************************
*/
/* Includes ------------------------------------------------------------------*/

#include "fft.h"
#include "display.h"
#include "DSPF_sp_fftSPxSP_cn.h"

float32_t aFFT_Input_f32[FFT_Length_Tab * 2];
float32_t aFFT_Output_f32 [FFT_Length_Tab * 2];

//float32_t FFT_Input_Q15_f[FFT_Length_Tab*2];
//q15_t aFFT_Input_Q15[FFT_Length_Tab*2];
//q15_t FFT_Output_Q15[FFT_Length_Tab];

//float32_t FFT_Input_Q31_f[FFT_Length_Tab*2];
//q31_t aFFT_Input_Q31[FFT_Length_Tab*2];
//q31_t FFT_Output_Q31[FFT_Length_Tab];

uint32_t mode = 0;

double MY_PI = 3.1415926535897932384626433832795;

float twiddle[FFT_Length_Tab * 2] = {0};
static void twiddleInit(int N_SAMPLES) {
    int i, j, k;
    double x_t, y_t, theta1, theta2, theta3;

    for (j = 1, k = 0; j <= N_SAMPLES >> 2; j = j << 2) {
        for (i = 0; i < N_SAMPLES >> 2; i += j) {
            theta1 = 2 * MY_PI * i / N_SAMPLES;
            x_t = cos(theta1);
            y_t = sin(theta1);
            twiddle[k] = (float) x_t;
            twiddle[k + 1] = (float) y_t;
            theta2 = 4 * MY_PI * i / N_SAMPLES;
            x_t = cos(theta2);
            y_t = sin(theta2);
            twiddle[k + 2] = (float) x_t;
            twiddle[k + 3] = (float) y_t;
            theta3 = 6 * MY_PI * i / N_SAMPLES;
            x_t = cos(theta3);
            y_t = sin(theta3);
            twiddle[k + 4] = (float) x_t;
            twiddle[k + 5] = (float) y_t;
            k += 6;
        }
    }
}

//float pTemp[FFT_Length_Tab];
float A[FFT_Length_Tab];
float B[FFT_Length_Tab];

void split_gen(float *pATable, float *pBTable, int n) {
    int i;

    for (i = 0; i < n; i++) {
        pATable[2 * i]   = 0.5 * (1.0  - sin(2 * MY_PI / (double)(2 * n) * (double) i));
        pATable[2 * i + 1] = 0.5 * (-1.0 * cos(2 * MY_PI / (double)(2 * n) * (double) i));
        pBTable[2 * i]   = 0.5 * (1.0  + sin(2 * MY_PI / (double)(2 * n) * (double) i));
        pBTable[2 * i + 1] = 0.5 * (1.0  * cos(2 * MY_PI / (double)(2 * n) * (double) i));
    }
}

void FFT_Split(int n, float *restrict pIn, float *pATable, float *pBTable, float *pOut) {
    int i;
    float Tr, Ti;
    pIn[2 * n] = pIn[0];
    pIn[2 * n + 1] = pIn[1];

    for (i = 0; i < n; i++) {
        Tr = (pIn[2 * i]   * pATable[2 * i] - pIn[2 * i + 1] * pATable[2 * i + 1] + pIn[2 * n - 2 * i] * pBTable[2 * i]   + pIn[2 * n - 2 * i + 1] * pBTable[2 * i + 1]);
        Ti = (pIn[2 * i + 1] * pATable[2 * i] + pIn[2 * i]   * pATable[2 * i + 1] + pIn[2 * n - 2 * i] * pBTable[2 * i + 1] - pIn[2 * n - 2 * i + 1] * pBTable[2 * i]);
        pOut[2 * i] = Tr;
        pOut[2 * i + 1] = Ti;
        // Use complex conjugate symmetry properties to get the rest..
        pOut[4 * n - 2 * i] = Tr;
        pOut[4 * n - 2 * i + 1] = -Ti;
    }

    pOut[2 * n] = pIn[0] - pIn[1];
    pOut[2 * n + 1] = 0;
}


/**
    @brief  This function Calculate FFT in Q15.
    @param  FFT Length : 1024, 256, 64
    @retval None
*/
void FFT_PROCESSING_Q15Process(uint32_t FFT_Length) {
    if (mode != 1) {
        mode = 1;
        twiddleInit(FFT_Length >> 1);
        split_gen(A, B, FFT_Length >> 1);
    }

    float32_t maxValue;    /* Max FFT value is stored here */
    uint32_t maxIndex;    /* Index in Output array where max value is */
    register uint32_t index_fill_input_buffer, index_fill_output_buffer, index_fill_adc_buffer = 0;
    uint32_t duration_us = 0x00;

    for (index_fill_adc_buffer = 0; index_fill_adc_buffer < FFT_Length * 2; index_fill_adc_buffer++) {
        aADC1ConvertedValue_s[index_fill_adc_buffer] = uhADCxConvertedValue;
        TIM2_Config();
    }

    TimerCount_Start();

    for (index_fill_input_buffer = 0, index_fill_adc_buffer = 0; index_fill_input_buffer < FFT_Length; ++index_fill_input_buffer, index_fill_adc_buffer += 2) {
        aFFT_Input_f32[index_fill_input_buffer] = (float32_t)aADC1ConvertedValue_s[index_fill_adc_buffer] / (float32_t)4096.0;
    }

    // calc N/2 FFT - 64 samples
    arm_cfft_radix4_instance_f32  FFT_F32_struct;
    arm_cfft_radix4_init_f32(&FFT_F32_struct, FFT_Length >> 1, FFT_INVERSE_FLAG, FFT_Normal_OUTPUT_FLAG);
    arm_cfft_radix4_f32(&FFT_F32_struct, aFFT_Input_f32);
    FFT_Split(FFT_Length >> 1, aFFT_Input_f32, A, B, aFFT_Output_f32);
    /* Process the data through the Complex Magniture Module for calculating the magnitude at each bin */
    arm_cmplx_mag_f32(aFFT_Output_f32, aFFT_Output_f32, FFT_Length);
    /* Calculates maxValue and returns corresponding value */
    arm_max_f32(aFFT_Output_f32, FFT_Length, &maxValue, &maxIndex);
    maxValue /= 200;
    TimerCount_Stop(nb_cycles);
    GUI_Clear();
    LCD_OUTPUT_Cycles(50, 260, nb_cycles);
    duration_us = (uint32_t)(((uint64_t)US_IN_SECOND * (nb_cycles)) / SystemCoreClock);
    LCD_OUTPUT_DURATION(220, 260, duration_us);

    for (index_fill_input_buffer = 0; index_fill_input_buffer < FFT_Length * 2; index_fill_input_buffer++) {
        GRAPH_DATA_YT_SetAlign(aGraph_Data[1], GRAPH_ALIGN_LEFT);
        GRAPH_DATA_YT_MirrorX(aGraph_Data[1], 1);
        GRAPH_DATA_YT_AddValue(aGraph_Data[1], aADC1ConvertedValue_s[index_fill_input_buffer] / 100 + 60);
    }

    for (index_fill_output_buffer = 0; index_fill_output_buffer < FFT_Length; index_fill_output_buffer++) {
        GRAPH_DATA_YT_SetAlign(aGraph_Data[0], GRAPH_ALIGN_LEFT);
        GRAPH_DATA_YT_MirrorX(aGraph_Data[0], 1);
        GRAPH_DATA_YT_AddValue(aGraph_Data[0], aFFT_Output_f32[index_fill_output_buffer] / maxValue + 1);
    }
}

/**
    @brief  This function Calculate FFT in F32.
    @param  FFT Length : 1024, 256, 64
    @retval None
*/
void FFT_PROCESSING_F32Process(uint32_t FFT_Length) {
    if (mode != 2) {
        mode = 2;
        twiddleInit(FFT_Length);
    }

    arm_cfft_radix2_instance_f32  FFT_F32_struct;
    float32_t maxValue;    /* Max FFT value is stored here */
    uint32_t maxIndex;    /* Index in Output array where max value is */
    uint32_t index_fill_input_buffer, index_fill_output_buffer, index_fill_adc_buffer = 0;
    uint32_t duration_us = 0x00;

    for (index_fill_adc_buffer = 0; index_fill_adc_buffer < FFT_Length * 2; index_fill_adc_buffer++) {
        aADC1ConvertedValue_s[index_fill_adc_buffer] = uhADCxConvertedValue;
        TIM2_Config();
    }

    TimerCount_Start();

    for (index_fill_input_buffer = 0; index_fill_input_buffer < FFT_Length * 2; index_fill_input_buffer += 2) {
        aFFT_Input_f32[index_fill_input_buffer] = (float32_t)aADC1ConvertedValue_s[index_fill_input_buffer] / (float32_t)4096.0;
        aFFT_Input_f32[index_fill_input_buffer + 1] = 0;   /* Imaginary part */
    }

    /* Initialize the CFFT/CIFFT module, intFlag = 0, doBitReverse = 1 */
    arm_cfft_radix2_init_f32(&FFT_F32_struct, FFT_Length, FFT_INVERSE_FLAG, FFT_Normal_OUTPUT_FLAG);
    arm_cfft_radix2_f32(&FFT_F32_struct, aFFT_Input_f32);
    /* Process the data through the Complex Magniture Module for calculating the magnitude at each bin */
    arm_cmplx_mag_f32(aFFT_Input_f32, aFFT_Output_f32, FFT_Length);
    /* Calculates maxValue and returns corresponding value */
    arm_max_f32(aFFT_Output_f32, FFT_Length, &maxValue, &maxIndex);
    maxValue /= 200;
    TimerCount_Stop(nb_cycles);
    GUI_Clear();
    LCD_OUTPUT_Cycles(50, 260, nb_cycles);
    duration_us = (uint32_t)(((uint64_t)US_IN_SECOND * (nb_cycles)) / SystemCoreClock);
    LCD_OUTPUT_DURATION(220, 260, duration_us);

    for (index_fill_input_buffer = 0; index_fill_input_buffer < FFT_Length * 2; index_fill_input_buffer++) {
        GRAPH_DATA_YT_SetAlign(aGraph_Data[1], GRAPH_ALIGN_LEFT);
        GRAPH_DATA_YT_MirrorX(aGraph_Data[1], 1);
        GRAPH_DATA_YT_AddValue(aGraph_Data[1], aADC1ConvertedValue_s[index_fill_input_buffer] / 100 + 60);
    }

    for (index_fill_output_buffer = 0; index_fill_output_buffer < FFT_Length; index_fill_output_buffer++) {
        GRAPH_DATA_YT_SetAlign(aGraph_Data[0], GRAPH_ALIGN_LEFT);
        GRAPH_DATA_YT_MirrorX(aGraph_Data[0], 1);
        GRAPH_DATA_YT_AddValue(aGraph_Data[0], aFFT_Output_f32[index_fill_output_buffer] / maxValue + 1);
    }
}

///**
//  * @brief  This function Calculate FFT in Q31.
//  * @param  FFT Length : 1024, 256, 64
//  * @retval None
//  */
void FFT_PROCESSING_Q31Process(uint32_t FFT_Length) {
//  arm_cfft_radix4_instance_q31  FFT_Q31_struct;
//
//  q31_t maxValue;    /* Max FFT value is stored here */
//  uint32_t maxIndex;    /* Index in Output array where max value is */
//
//  uint32_t index_fill_input_buffer, index_fill_output_buffer, index_fill_adc_buffer = 0;
//  uint32_t duration_us = 0x00;
//
//  for (index_fill_adc_buffer = 0; index_fill_adc_buffer < FFT_Length*2; index_fill_adc_buffer ++)
//  {
//    aADC1ConvertedValue_s[index_fill_adc_buffer] = uhADCxConvertedValue;
//    TIM2_Config();
//  }
//  for (index_fill_input_buffer = 0; index_fill_input_buffer < FFT_Length*2; index_fill_input_buffer += 2)
//  {
//    FFT_Input_Q31_f[(uint16_t)index_fill_input_buffer] = (float32_t)uhADCxConvertedValue / (float32_t)4096.0;
//    /* Imaginary part */
//    FFT_Input_Q31_f[(uint16_t)(index_fill_input_buffer + 1)] = 0;
//
//    TIM2_Config();
//  }
//
//  arm_float_to_q31((float32_t *)&FFT_Input_Q31_f[0], (q31_t *)&aFFT_Input_Q31[0], FFT_Length*2);
//
//  /* Initialize the CFFT/CIFFT module, intFlag = 0, doBitReverse = 1 */
//  arm_cfft_radix4_init_q31(&FFT_Q31_struct, FFT_Length, FFT_INVERSE_FLAG, FFT_Normal_OUTPUT_FLAG);
//
//  TimerCount_Start();
//  arm_cfft_radix4_q31(&FFT_Q31_struct, aFFT_Input_Q31);
//  TimerCount_Stop(nb_cycles);
//
//  GUI_Clear();
//  LCD_OUTPUT_Cycles(50, 260, nb_cycles);
//  duration_us = (uint32_t)(((uint64_t)US_IN_SECOND * (nb_cycles)) / SystemCoreClock);
//  LCD_OUTPUT_DURATION(220, 260, duration_us);
//
//  /* Process the data through the Complex Magniture Module for calculating the magnitude at each bin */
//  arm_cmplx_mag_q31(aFFT_Input_Q31, FFT_Output_Q31, FFT_Length);
//
//  /* Calculates maxValue and returns corresponding value */
//  arm_max_q31(FFT_Output_Q31, FFT_Length, &maxValue, &maxIndex);
//  maxValue = 0;
//
//  for (index_fill_input_buffer = 0; index_fill_input_buffer < FFT_Length*2; index_fill_input_buffer++)
//  {
//    GRAPH_DATA_YT_SetAlign(aGraph_Data[1], GRAPH_ALIGN_LEFT);
//    GRAPH_DATA_YT_MirrorX (aGraph_Data[1], 1);
//    GRAPH_DATA_YT_AddValue(aGraph_Data[1], aADC1ConvertedValue_s[index_fill_input_buffer] / 100 + 50);
//  }
//  for (index_fill_output_buffer = 0; index_fill_output_buffer < FFT_Length; index_fill_output_buffer++)
//  {
//    GRAPH_DATA_YT_SetAlign(aGraph_Data[0], GRAPH_ALIGN_LEFT);
//    GRAPH_DATA_YT_MirrorX (aGraph_Data[0], 1);
//    GRAPH_DATA_YT_AddValue(aGraph_Data[0], FFT_Output_Q31[index_fill_output_buffer] / 5376212 + 10);
//  }
}
//
///**
//  * @brief  This function Calculate FFT in Q15.
//  * @param  FFT Length : 1024, 256, 64
//  * @retval None
//  */
//void FFT_PROCESSING_Q15Process(uint32_t FFT_Length)
//{
//
//  arm_cfft_radix4_instance_q15  FFT_Q15_struct;
//
//  q15_t maxValue;    /* Max FFT value is stored here */
//  uint32_t maxIndex;    /* Index in Output array where max value is */
//
//  uint32_t index_fill_input_buffer, index_fill_output_buffer, index_fill_adc_buffer = 0;
//  uint32_t duration_us = 0x00;
//  for (index_fill_adc_buffer = 0; index_fill_adc_buffer < FFT_Length*2; index_fill_adc_buffer ++)
//  {
//    aADC1ConvertedValue_s[index_fill_adc_buffer] =uhADCxConvertedValue;
//
//    TIM2_Config();
//  }
//  for (index_fill_input_buffer = 0; index_fill_input_buffer < FFT_Length*2; index_fill_input_buffer += 2)
//  {
//    FFT_Input_Q15_f[(uint16_t)index_fill_input_buffer] = (float32_t)uhADCxConvertedValue / (float32_t)4096.0;
//    /* Imaginary part */
//    FFT_Input_Q15_f[(uint16_t)(index_fill_input_buffer + 1)] = 0;
//
//    TIM2_Config();
//  }
//
//  arm_float_to_q15((float32_t *)&FFT_Input_Q15_f[0], (q15_t *)&aFFT_Input_Q15[0], FFT_Length*2);
//
//  /* Initialize the CFFT/CIFFT module, intFlag = 0, doBitReverse = 1 */
//  arm_cfft_radix4_init_q15(&FFT_Q15_struct, FFT_Length, FFT_INVERSE_FLAG, FFT_Normal_OUTPUT_FLAG);
//
//  TimerCount_Start();
//  arm_cfft_radix4_q15(&FFT_Q15_struct, aFFT_Input_Q15);
//  TimerCount_Stop(nb_cycles);
//
//  GUI_Clear();
//  LCD_OUTPUT_Cycles(50, 260, nb_cycles);
//  duration_us = (uint32_t)(((uint64_t)US_IN_SECOND * (nb_cycles)) / SystemCoreClock);
//  LCD_OUTPUT_DURATION(220, 260, duration_us);
//
//  /* Process the data through the Complex Magniture Module for calculating the magnitude at each bin */
//  arm_cmplx_mag_q15(aFFT_Input_Q15, FFT_Output_Q15, FFT_Length);
//
//  /* Calculates maxValue and returns corresponding value */
//  arm_max_q15(FFT_Output_Q15, FFT_Length, &maxValue, &maxIndex);
//  maxValue = 0;
//
//  for (index_fill_input_buffer = 0; index_fill_input_buffer < FFT_Length*2; index_fill_input_buffer++)
//  {
//    GRAPH_DATA_YT_SetAlign(aGraph_Data[1], GRAPH_ALIGN_LEFT);
//    GRAPH_DATA_YT_MirrorX (aGraph_Data[1], 1);
//    GRAPH_DATA_YT_AddValue(aGraph_Data[1], aADC1ConvertedValue_s[index_fill_input_buffer] / 100 + 50);
//  }
//  for (index_fill_output_buffer = 0; index_fill_output_buffer < FFT_Length; index_fill_output_buffer++)
//  {
//    GRAPH_DATA_YT_SetAlign(aGraph_Data[0], GRAPH_ALIGN_LEFT);
//    GRAPH_DATA_YT_MirrorX (aGraph_Data[0], 1);
//    GRAPH_DATA_YT_AddValue(aGraph_Data[0], FFT_Output_Q15[index_fill_output_buffer] / 50 + 10);
//
//  }
//}

/************************ (C) COPYRIGHT STMicroelectronics ************************/
