/**
  ******************************************************************************
  * File Name   : main.c
  * Description :
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "math.h"
#include "lcd_log.h"
#include "arm_math.h"
#include "color_waves.h"
#include "stm32746g_discovery_lcd.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define NUM_TAPS    128
#define COR_SAMPLES NUM_SAMPLES
// #define COR_SAMPLES  16
#define INTERPOLATION_FACTOR 4
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
float iirCoeff[12] = {
  0.335017, 0.000000, 0.390203, 0.335017, -0.169458, 0.229694,
  0.335017, 0.000000, 0.390203, 0.335017, -0.169458, 0.229694
};

q15_t firCoeff[128] = {
  0, 0, 0, 0, 0, -1, -3, -5, -7, -9, -10, -10, -8, -3, 4, 13, 23, 33, 42, 47, 48, 43, 31, 12, -13, -41, -71, -98, -118, -129, -127, -109, -76, -28, 31, 96, 162, 220, 263, 284, 276, 236, 164, 61, -66, -206, -347, -474, -572, -623, -614, -535, -379, -145, 162, 533, 950, 1393, 1839, 2261, 2635, 2939, 3152, 3263, 3263, 3152, 2939, 2635, 2261, 1839, 1393, 950, 533, 162, -145, -379, -535, -614, -623, -572, -474, -347, -206, -66, 61, 164, 236, 276, 284, 263, 220, 162, 96, 31, -28, -76, -109, -127, -129, -118, -98, -71, -41, -13, 12, 31, 43, 48, 47, 42, 33, 23, 13, 4, -3, -8, -10, -10, -9, -7, -5, -3, -1, 0, 0, 0, 0, 0,
};


OSC_TypeDef oscConfig;

TIM_HandleTypeDef hTim2;
volatile uint32_t result = 0;
volatile uint32_t uwVolume = 70;
volatile uint32_t length = NUM_SAMPLES;
uint32_t array1[NUM_SAMPLES];
uint32_t array2[NUM_SAMPLES];
uint32_t offset = 0;
int8_t bufferMic = 0;
q15_t pcmBuff[NUM_SAMPLES * 4]; // 2 channels, 2 buffers (double buffering)
q15_t pcmBuffOut[NUM_SAMPLES * 4]; // 2 channels, 2 buffers (double buffering)
q15_t left[NUM_SAMPLES];
q15_t leftComplex[2 * NUM_SAMPLES];
q15_t leftFFT[2 * NUM_SAMPLES];
float auxFloat[2 * NUM_SAMPLES];
q15_t leftFFTAbs[NUM_SAMPLES];
q15_t right[NUM_SAMPLES];
q15_t corr[NUM_SAMPLES * 2];
q15_t pFIRStateLeft[NUM_TAPS + NUM_SAMPLES];
q15_t pFIRStateRight[NUM_TAPS + NUM_SAMPLES];
float pIIRStateLeft[4 * 2];
float pIIRStateRight[4 * 2]; float CpuLoad = 0;
arm_rfft_instance_q15 Sfft;

volatile uint32_t time_start, time_end, time_diff;
volatile int outputBufState = -1;
volatile q15_t  maxCorr;
volatile int32_t maxCorrIndex = 0, maxCorrIndexLiveWatch = 0;
arm_fir_instance_q15 SFIRleft, SFIRright;
arm_biquad_casd_df1_inst_f32 SIIRleft, SIIRright;
volatile uint32_t cntr = 0;
TEXT_Handle hText, hText2, hText3;
char resultText[20] = "init";
char textLoad[30] = "init";
char textCycles[30] = "init";

volatile int32_t doCorr = 1, doFiltIIR = 0, doFiltFIR = 0, doFFT = 0, doInter = 0;

/* Private function prototypes -----------------------------------------------*/
static void SystemClock_Config(void);
static void Error_Handler(void);
static void MPU_Config(void);
static uint8_t PlayerInit(uint32_t AudioFreq);
static void CPU_CACHE_ENABLE(void);
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void) {
  uint32_t  j;
  float  power;
  length = NUM_SAMPLES;// this varible keeps the legth of array for memory save macro
  HAL_Init();
  CPU_CACHE_ENABLE();
  SystemClock_Config();
  ScopeLCD_Init();
  hText = TEXT_Create(LCD_GetXSize() - 170, 10, 160, 30,  GUI_ID_TEXT0, WM_CF_SHOW, resultText, 0);
  hText2 = TEXT_Create(LCD_GetXSize() - 170, 30, 160, 30,  GUI_ID_TEXT1, WM_CF_SHOW, textCycles, 0);
  hText3 = TEXT_Create(LCD_GetXSize() - 170, 50, 160, 30,  GUI_ID_TEXT2, WM_CF_SHOW, textLoad, 0);
  color_wave_initialize();
  oscConfig.numberOfSamples = NUM_SAMPLES;
  AUDIO_IO_Init();
  BSP_AUDIO_IN_OUT_Init(INPUT_DEVICE_DIGITAL_MICROPHONE_2, OUTPUT_DEVICE_HEADPHONE, DEFAULT_AUDIO_IN_FREQ, DEFAULT_AUDIO_IN_BIT_RESOLUTION, DEFAULT_AUDIO_IN_CHANNEL_NBR);
  BSP_AUDIO_IN_Record((uint16_t *)pcmBuff, 4 * NUM_SAMPLES);
  /* Start Playback */
  BSP_AUDIO_OUT_SetAudioFrameSlot(CODEC_AUDIOFRAME_SLOT_02);
  BSP_AUDIO_OUT_Play((uint16_t *)pcmBuffOut, 8 * NUM_SAMPLES);
  // initialize the FIR filters
  arm_fir_init_q15(&SFIRleft, NUM_TAPS, firCoeff, pFIRStateLeft, NUM_SAMPLES);
  arm_fir_init_q15(&SFIRright, NUM_TAPS, firCoeff, pFIRStateRight, NUM_SAMPLES);
  arm_biquad_cascade_df1_init_f32(&SIIRleft, 2, iirCoeff, pIIRStateLeft);
  arm_biquad_cascade_df1_init_f32(&SIIRright, 2, iirCoeff, pIIRStateRight);
  // initialization of the FFT
  arm_rfft_init_q15(&Sfft, NUM_SAMPLES, 0, 1);

  while (1) {
    if (bufferMic) { // any buffer full
      DEBUG_PIN_HIGH;

      if (bufferMic == 1) { // which one?
        offset = 0;
      } else if (bufferMic == 2) {
        offset = 2 * NUM_SAMPLES;
      }

      bufferMic = 0;  // clear the buffer status

      // extract left and right
      for (j = 0; j < NUM_SAMPLES; j++) {
        right[j] = pcmBuff[offset + j * 2]  ;       // scale down to avoid overflow in correlation
        left[j] = leftComplex[j * 2] = pcmBuff[offset + j * 2 + 1] ;
      }

      TIM_MEASURE_START;

      if (doInter) {
        for (j = 0; j < NUM_SAMPLES; j++) {
          right[j] = left[j] = 0;  // clean the arrays first
        }

        for (j = 0; j < NUM_SAMPLES / INTERPOLATION_FACTOR; j++) {
          right[j * INTERPOLATION_FACTOR] = pcmBuff[offset + j * 2]  * 4;
          left[j * INTERPOLATION_FACTOR] = leftComplex[j * 2] = pcmBuff[offset + j * 2 + 1] * 4; // take only Nth sample
        }
      }

      if (doFiltFIR) { // make FIR if requested by doFiltFIR flag or in case of interplation we need to filter out zeros
        arm_fir_fast_q15(&SFIRleft, left, left, NUM_SAMPLES);
        arm_fir_fast_q15(&SFIRright, right, right, NUM_SAMPLES);
      }

      if (doFiltIIR) { // the IIRs are very sensitive to rounding and truncation. To make this particular filter working we must convert to float and back to q15
        arm_q15_to_float(left, auxFloat, NUM_SAMPLES);  // doing IIR is very sensitive to rouding, better to be done in float
        arm_biquad_cascade_df1_f32(&SIIRleft, auxFloat, auxFloat, NUM_SAMPLES);   // filter in float
        arm_float_to_q15(auxFloat, left, NUM_SAMPLES);  // and convert back to q15
        arm_q15_to_float(right, auxFloat, NUM_SAMPLES);  // doing IIR is very sensitive to rouding, better to be done in float
        arm_biquad_cascade_df1_f32(&SIIRright, auxFloat, auxFloat, NUM_SAMPLES);   // filter in float
        arm_float_to_q15(auxFloat, right, NUM_SAMPLES);  // and convert back to q15
      }

      if (doCorr) {
        arm_correlate_fast_q15(left, COR_SAMPLES, right, COR_SAMPLES, corr);
      }

      if (doFFT) { // the absolute value is very sensitive to rounding and truncation. it's better to convert to float and then back.
        arm_rfft_q15(&Sfft, left, leftFFT);
        arm_q15_to_float(leftFFT, auxFloat, 2 * NUM_SAMPLES); // doing abs is very sensitive to rounding, better to be done in float
        arm_cmplx_mag_f32(auxFloat, auxFloat, NUM_SAMPLES);
        arm_float_to_q15(auxFloat, left, NUM_SAMPLES);
      }

      TIM_MEASURE_END;
      DEBUG_PIN_LOW;
      YOU_CAN_PUT_BREAKPOINT_HERE;
      // find maximum
      {
        maxCorr = 0;

        for (j = 0; j < NUM_SAMPLES * 2 - 1; j++) {
          if (corr[j] > maxCorr) {
            maxCorr = corr[j];
            maxCorrIndex = j;
          }
        }
      }

      // playback the signals to headphones
      for (j = 0; j < NUM_SAMPLES; j++) {
        pcmBuffOut[offset + j * 2] = right[j] * 4;
        pcmBuffOut[offset + j * 2 + 1] = left[j] * 4;
      }

      // calculate RMS value to display by wave
      arm_q15_to_float(right, auxFloat, NUM_SAMPLES);
      arm_rms_f32(auxFloat, NUM_SAMPLES, &power);

      if (power > 1.0f) {
        power = 1.0f;
      }

      // now show the results
      if (power > 0.001f) {
        if ((maxCorrIndex - COR_SAMPLES + 1 >= -(doInter * (INTERPOLATION_FACTOR - 1) + 1)) && (maxCorrIndex - COR_SAMPLES + 1 <= (doInter * (INTERPOLATION_FACTOR - 1) + 1))) { // check if it's in valid range
          maxCorrIndexLiveWatch =  maxCorrIndex - COR_SAMPLES + 1;  // latch the actual max value and substract the offset
        }
      } else {
        maxCorrIndexLiveWatch = 0;
      }

      sprintf(resultText, "Delay is: %d", maxCorrIndexLiveWatch);
      TEXT_SetText(hText, resultText);
      sprintf(textCycles, "CPU cycles in DSP: %d", time_diff);
      TEXT_SetText(hText2, textCycles);
      CpuLoad = (float)(100.0 * (float)time_diff * (DEFAULT_AUDIO_IN_FREQ / NUM_SAMPLES) / (float)HAL_RCC_GetSysClockFreq());
      //CpuLoad -=0.5;
      sprintf(textLoad, "CPU load in DSP: %f %%", CpuLoad);
      TEXT_SetText(hText3, textLoad);
      ScopeLCD_Show(right, left);
      WaveDisplay((maxCorrIndexLiveWatch * 1024) / (doInter * (INTERPOLATION_FACTOR - 1) + 1), (int16_t)(power * 32767.0f));
    }
  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 200000000
  *            HCLK(Hz)                       = 200000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 400
  *            PLL_P                          = 2
  *            PLL_Q                          = 8
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 6
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void) {
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();
  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 400;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 8;
  HAL_RCC_OscConfig(&RCC_OscInitStruct);
  /* activate the OverDrive to reach the 180 Mhz Frequency */
  HAL_PWREx_ActivateOverDrive();
  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2
     clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

/**
  * @brief  Calculates the remaining file size and new position of the pointer.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_TransferComplete_CallBack(void) {
  outputBufState = 1;
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_OUT_HalfTransfer_CallBack(void) {
  outputBufState = 0;
}

/**
  * @brief  Manages the DMA Half Transfer complete interrupt.
  * @param  None
  * @retval None
  */
void BSP_AUDIO_IN_HalfTransfer_CallBack(void) {
  bufferMic = 1;
}

/**
  * @brief Manages the DMA Transfer complete interrupt.
  * @param None
  * @retval None
  */
void BSP_AUDIO_IN_TransferComplete_CallBack(void) {
  bufferMic = 2;
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @param  None
  * @retval None
  */
static void Error_Handler(void) {
  /* User may add here some code to deal with this error */
  while (1) {
  }
}

static GPIO_InitTypeDef  GPIO_InitStruct;
void HAL_MspInit(void) {
  __HAL_RCC_GPIOI_CLK_ENABLE();
  GPIO_InitStruct.Pin = (GPIO_PIN_3);
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FAST;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
  __HAL_RCC_TIM2_CLK_ENABLE();
  __HAL_RCC_TIMCLKPRESCALER(RCC_TIMPRES_ACTIVATED);
  hTim2.Instance = TIM2;
  hTim2.Instance->ARR = 0xFFFFFFFF;
  __HAL_TIM_ENABLE(&hTim2);
}

/**
  * @brief  Configure the MPU attributes as Write Through for SRAM1/2.
  * @note   The Base Address is 0x20010000 since this memory interface is the AXI.
  *         The Region Size is 256KB, it is related to SRAM1 and SRAM2  memory size.
  * @param  None
  * @retval None
  */
#ifdef ARM_MATH_CM7
static void MPU_Config(void) {
  MPU_Region_InitTypeDef MPU_InitStruct;
  /* Disable the MPU */
  HAL_MPU_Disable();
  /* Configure the MPU attributes as WT for SRAM */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x20010000;
  MPU_InitStruct.Size = MPU_REGION_SIZE_256KB;
  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x00;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;
  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_ENABLE(void) {
  (*(uint32_t *) 0xE000ED94) &= ~0x5;
  (*(uint32_t *) 0xE000ED98) = 0x0; //MPU->RNR
  (*(uint32_t *) 0xE000ED9C) = 0x20010000 | 1 << 4; //MPU->RBAR
  (*(uint32_t *) 0xE000EDA0) = 0 << 28 | 3 << 24 | 0 << 19 | 0 << 18 | 1 << 17 | 0 << 16 | 0 << 8 | 30 << 1 | 1 << 0 ; //MPU->RASE  WT
  (*(uint32_t *) 0xE000ED94) = 0x5;
  /* Invalidate I-Cache : ICIALLU register*/
  SCB_InvalidateICache();
  /* Enable branch prediction */
  SCB->CCR |= (1 << 18);
  __DSB();
  /* Enable I-Cache */
  SCB_EnableICache();
  /* Enable D-Cache */
  SCB_InvalidateDCache();
  SCB_EnableDCache();
}
#endif

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line) {
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1) {
  }
}
#endif
