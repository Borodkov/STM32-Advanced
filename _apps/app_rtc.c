/**
  ******************************************************************************
    @file
    @author  PavelB
    @version V1.0
    @date    03-April-2017
    @brief
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "lcd_log.h"
#include "cpu_utils.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;
RTC_TimeTypeDef time;
RTC_DateTypeDef date;
float temperature;
ADC_HandleTypeDef hadc1;

uint32_t adcTempSensorValue = 0;

/* Private function prototypes -----------------------------------------------*/
void MX_RTC_Init(void);
void MX_ADC1_Init(void);

/**
    @brief  Timer callback that update RTC vars
    @param  argument not used
    @retval None
*/
static void osRTCTimerCallback(void const *argument) {
    (void) argument;
    /***************************************************************************
        every second update RTC vars
    ***************************************************************************/
    HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
    /***************************************************************************
        show on LCD
    ***************************************************************************/
    char sFooter[50] = {0};
    sprintf(sFooter, "Press BTN to switch | Load %02u% | T = %+.1f [C]\n", osGetCPUUsage(), temperature);
    LCD_LOG_SetFooter((uint8_t *)sFooter);
}

/**
    @brief  Timer callback that update RTC vars
    @param  argument not used
    @retval None
*/
static void osTemperatureTimerCallback(void const *argument) {
    (void) argument;
    /***************************************************************************
        every second check chip temperature
    ***************************************************************************/
    float Vsense = 0, V25 = 0.76, Avg_Slope = 0.0025;
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);

    /* Check if the continous conversion of regular channel is finished */
    if (HAL_IS_BIT_SET(HAL_ADC_GetState(&hadc1), HAL_ADC_STATE_REG_EOC)) {
        /*##-5- Get the converted value of regular channel  ########################*/
        adcTempSensorValue = HAL_ADC_GetValue(&hadc1);
    }

    // formula from DS on stm32f42xx
    Vsense = 3.33 * ((float)adcTempSensorValue / 4096);
    temperature = ((Vsense - V25) / Avg_Slope) + 25;
}

/*----------------------------------------------------------------------------*/
void vStartTimers() {
    LCD_UsrLog("vStartTimers()\n");
    MX_RTC_Init();
    MX_ADC1_Init();
    /***************************************************************************
        Set default date and time
    ***************************************************************************/
    date.Year    = (uint8_t)17;
    date.Month   = RTC_MONTH_AUGUST;
    date.Date    = ((uint8_t)26);
    date.WeekDay = RTC_WEEKDAY_SATURDAY;
    time.Hours   = (uint8_t)7;
    time.Minutes = (uint8_t)8;
    time.Seconds = (uint8_t)9;
    HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
    HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
    /* Create Timer */
    osTimerDef(timRTC, osRTCTimerCallback);
    osTimerId osTimerRTC = osTimerCreate(osTimer(timRTC), osTimerPeriodic, NULL);
    osTimerDef(timTemperature, osTemperatureTimerCallback);
    osTimerId osTimerTemperature = osTimerCreate(osTimer(timTemperature), osTimerPeriodic, NULL);
    /* Start Timer */
    osTimerStart(osTimerRTC, 1000);
    osTimerStart(osTimerTemperature, 10000);
}

/* RTC init function */
void MX_RTC_Init(void) {
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;
    /** Initialize RTC Only
    */
    hrtc.Instance = RTC;
    hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv = 255;
    hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;

    if (HAL_RTC_Init(&hrtc) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    /** Initialize RTC and set the Time and Date
    */
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR0) != 0x32F2) {
        sTime.Hours = 0x0;
        sTime.Minutes = 0x0;
        sTime.Seconds = 0x0;
        sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
        sTime.StoreOperation = RTC_STOREOPERATION_RESET;

        if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK) {
            _Error_Handler(__FILE__, __LINE__);
        }

        sDate.WeekDay = RTC_WEEKDAY_MONDAY;
        sDate.Month = RTC_MONTH_JANUARY;
        sDate.Date = 0x1;
        sDate.Year = 0x0;

        if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK) {
            _Error_Handler(__FILE__, __LINE__);
        }

        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR0, 0x32F2);
    }
}

void HAL_RTC_MspInit(RTC_HandleTypeDef *rtcHandle) {
    if (rtcHandle->Instance == RTC) {
        __HAL_RCC_RTC_ENABLE();
    }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef *rtcHandle) {
    if (rtcHandle->Instance == RTC) {
        __HAL_RCC_RTC_DISABLE();
    }
}
/* ADC1 init function */
void MX_ADC1_Init(void) {
    ADC_ChannelConfTypeDef sConfig;
    /* Configure the ADC peripheral */
    hadc1.Instance          = ADC1;
    hadc1.Init.ClockPrescaler        = ADC_CLOCKPRESCALER_PCLK_DIV4;
    hadc1.Init.Resolution            = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode          = DISABLE;                       /* Sequencer disabled (ADC conversion on only 1 channel: channel set on rank 1) */
    hadc1.Init.ContinuousConvMode    = ENABLE;                        /* Continuous mode enabled to have continuous conversion */
    hadc1.Init.DiscontinuousConvMode = DISABLE;                       /* Parameter discarded because sequencer is disabled */
    hadc1.Init.NbrOfDiscConversion   = 0;
    hadc1.Init.ExternalTrigConvEdge  = ADC_EXTERNALTRIGCONVEDGE_NONE;        /* Conversion start not trigged by an external event */
    hadc1.Init.ExternalTrigConv      = ADC_EXTERNALTRIGCONV_T1_CC1;
    hadc1.Init.DataAlign             = ADC_DATAALIGN_RIGHT;
    hadc1.Init.NbrOfConversion       = 1;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection          = DISABLE;

    if (HAL_ADC_Init(&hadc1) != HAL_OK) {
        /* ADC initialization Error */
        Error_Handler();
    }

    /* Configure ADC Temperature Sensor Channel */
    sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_56CYCLES;
    sConfig.Offset = 0;

    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}
