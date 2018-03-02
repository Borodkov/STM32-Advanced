/**
  ******************************************************************************
  * @file
  * @author  PavelB
  * @version V1.0
  * @date    03-April-2017
  * @brief
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Extern function prototypes ------------------------------------------------*/
extern void stringCatAndView(char *str);

/* Extern variables ----------------------------------------------------------*/
extern RTC_TimeTypeDef time;
extern RTC_DateTypeDef date;

/* Private define ------------------------------------------------------------*/
#define ADCx_CLK_ENABLE()               __HAL_RCC_ADC3_CLK_ENABLE()
#define DMAx_CLK_ENABLE()               __HAL_RCC_DMA2_CLK_ENABLE()
#define ADCx_CHANNEL_GPIO_CLK_ENABLE()  __HAL_RCC_GPIOF_CLK_ENABLE()

/* Definition for ADCx Channel Pin */
#define ADCx_CHANNEL_PIN                GPIO_PIN_10
#define ADCx_CHANNEL_GPIO_PORT          GPIOF
/* Definition for ADCx's DMA */
#define ADCx_DMA_CHANNEL                DMA_CHANNEL_2
#define ADCx_DMA_STREAM                 DMA2_Stream0

/* Definition for ADCx's NVIC */
#define ADCx_DMA_IRQn                   DMA2_Stream0_IRQn
#define ADCx_DMA_IRQHandler             DMA2_Stream0_IRQHandler

#define  ADC_WINDOW  1000
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;
TIM_HandleTypeDef htim2;

static osSemaphoreId xBinSem_ADCdoneHandle = NULL;
osThreadId ADCHandle;

uint16_t adcBuf[ADC_WINDOW][6] = {1};
uint32_t adcBufMean[6];

static char s[100] = {0};

/* Private function prototypes -----------------------------------------------*/
void MX_DMA_Init(void);
void MX_ADC3_Init(void);
void MX_TIM2_Init(void);

/**
  * @brief
  * @param
  * @retval None
  */
static void task(void const *pvParameters) {
    //osThreadSuspend(NULL);
    /***************************************************************************
    Toggle LED3 every ADC conversion, and LED4 - when DMA complete (ADC_WINDOW conversions)
    ***************************************************************************/
    HAL_ADC_Start_DMA(&hadc3, (uint32_t*)adcBuf, 6 * ADC_WINDOW);
    __HAL_RCC_DAC_CLK_ENABLE(); // errata of stm32f7 chip
    HAL_TIM_Base_Start(&htim2);

    /* Infinite loop */
    for (;;) {
        osSemaphoreWait(xBinSem_ADCdoneHandle, osWaitForever); // w8 semaphore forever

        /***************************************************************************
        Computation of ADC conversions raw data to physical values
        ***************************************************************************/
        for (uint32_t n = 0; n < 6; ++n) {
            adcBufMean[n] = 0;

            for (uint32_t i = 0; i < ADC_WINDOW; ++i) {
                adcBufMean[n] += adcBuf[i][n];
            } adcBufMean[n] /= ADC_WINDOW;
        }

        /***************************************************************************
        Display the Temperature Value on the LCD
        ***************************************************************************/
        sprintf(s, "%04u | %04u | %04u | %04u | %04u | %04u |\n",
                adcBufMean[0], adcBufMean[1], adcBufMean[2],
                adcBufMean[3], adcBufMean[4], adcBufMean[5]);
        LCD_UsrLog(s);

        // is this damage ?
        if (adcBufMean[0] >= 4000 || adcBufMean[0] < 100 ||
                adcBufMean[1] >= 4000 || adcBufMean[1] < 100 ||
                adcBufMean[2] >= 4000 || adcBufMean[2] < 100 ||
                adcBufMean[3] >= 4000 || adcBufMean[3] < 100 ||
                adcBufMean[4] >= 4000 || adcBufMean[4] < 100 ||
                adcBufMean[5] >= 4000 || adcBufMean[5] < 100) {
            BSP_LCD_SetTextColor(LCD_COLOR_RED);
            BSP_LCD_FillRect(0, 0, BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
            BSP_LCD_SetFont(&Font24);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_SetBackColor(LCD_COLOR_RED);
            BSP_LCD_DisplayStringAt(0, BSP_LCD_GetYSize() / 2, "DAMAGE", CENTER_MODE);
            // send string DAMAGE
            stringCatAndView("DAMAGE\n");
            osDelay(100);
            // pause
            osThreadSuspendAll();
            // w8
            HAL_Delay(3333);
            // restore disp
            LCD_LOG_Init();
            LCD_LOG_SetHeader("LAN 6 channels ADC 50 kHz Damage Analyzer");
            LCD_LOG_SetFooter("Press BTN to switch | Load 00%");
            // go go go
            osThreadResumeAll();
        }
    }
}

/*----------------------------------------------------------------------------*/
void vStartADCTask() {
    LCD_UsrLog("vStartADCTask()\n");
    MX_DMA_Init();
    MX_ADC3_Init();
    MX_TIM2_Init();
    /* definition and creation of xBinSem_... */
    osSemaphoreDef(xBinSem_ADCdone);
    xBinSem_ADCdoneHandle = osSemaphoreCreate(osSemaphore(xBinSem_ADCdone), 1);
    /* take semaphore ... */
    osSemaphoreWait(xBinSem_ADCdoneHandle, 0);
    /* Create that task */
    osThreadDef(ADC,
                task,
                osPriorityNormal,
                0,
                2 * configMINIMAL_STACK_SIZE);
    ADCHandle = osThreadCreate(osThread(ADC), NULL);
}

/* ADC3 init function */
void MX_ADC3_Init(void) {
    ADC_ChannelConfTypeDef sConfig;
    /* Configure the global features of the ADC
      (Clock, Resolution, Data Alignment and number of conversion) */
    hadc3.Instance = ADC3;
    hadc3.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc3.Init.Resolution = ADC_RESOLUTION_12B;
    hadc3.Init.ScanConvMode = ENABLE;
    hadc3.Init.ContinuousConvMode = DISABLE;
    hadc3.Init.DiscontinuousConvMode = DISABLE;
    hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_TRGO;
    hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc3.Init.NbrOfConversion = 6;
    hadc3.Init.DMAContinuousRequests = ENABLE;
    hadc3.Init.EOCSelection = ADC_EOC_SEQ_CONV;

    if (HAL_ADC_Init(&hadc3) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    /* Configure for the selected ADC regular channel its corresponding rank
       in the sequencer and its sample time. */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;

    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sConfig.Channel = ADC_CHANNEL_8;
    sConfig.Rank = 2;

    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sConfig.Channel = ADC_CHANNEL_7;
    sConfig.Rank = 3;

    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sConfig.Channel = ADC_CHANNEL_6;
    sConfig.Rank = 4;

    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sConfig.Channel = ADC_CHANNEL_5;
    sConfig.Rank = 5;

    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sConfig.Channel = ADC_CHANNEL_4;
    sConfig.Rank = 6;

    if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}
void HAL_ADC_MspInit(ADC_HandleTypeDef* adcHandle) {
    GPIO_InitTypeDef GPIO_InitStruct;

    if (adcHandle->Instance == ADC1) {
        /* Peripheral clock enable */
        __HAL_RCC_ADC1_CLK_ENABLE();
    }

    if (adcHandle->Instance == ADC3) {
        __HAL_RCC_ADC3_CLK_ENABLE();
        /**ADC3 GPIO Configuration
        PF7      ------> ADC3_IN5
        PF6      ------> ADC3_IN4
        PF10     ------> ADC3_IN8
        PF9      ------> ADC3_IN7
        PF8      ------> ADC3_IN6
        PA0/WKUP ------> ADC3_IN0 */
        GPIO_InitStruct.Pin = ARDUINO_A4_Pin | ARDUINO_A5_Pin | ARDUINO_A1_Pin | ARDUINO_A2_Pin | ARDUINO_A3_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
        GPIO_InitStruct.Pin = ARDUINO_A0_Pin;
        GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        HAL_GPIO_Init(ARDUINO_A0_GPIO_Port, &GPIO_InitStruct);
        /* ADC3 DMA Init */
        hdma_adc3.Instance = DMA2_Stream1;
        hdma_adc3.Init.Channel = DMA_CHANNEL_2;
        hdma_adc3.Init.Direction = DMA_PERIPH_TO_MEMORY;
        hdma_adc3.Init.PeriphInc = DMA_PINC_DISABLE;
        hdma_adc3.Init.MemInc = DMA_MINC_ENABLE;
        hdma_adc3.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
        hdma_adc3.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
        hdma_adc3.Init.Mode = DMA_CIRCULAR;
        hdma_adc3.Init.Priority = DMA_PRIORITY_HIGH;
        hdma_adc3.Init.FIFOMode = DMA_FIFOMODE_DISABLE;

        if (HAL_DMA_Init(&hdma_adc3) != HAL_OK) {
            _Error_Handler(__FILE__, __LINE__);
        }

        __HAL_LINKDMA(adcHandle, DMA_Handle, hdma_adc3);
    }
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* adcHandle) {
    if (adcHandle->Instance == ADC1) {
        __HAL_RCC_ADC1_CLK_DISABLE();
    }

    if (adcHandle->Instance == ADC3) {
        __HAL_RCC_ADC3_CLK_DISABLE();
        /**ADC3 GPIO Configuration
        PF7      ------> ADC3_IN5
        PF6      ------> ADC3_IN4
        PF10     ------> ADC3_IN8
        PF9      ------> ADC3_IN7
        PF8      ------> ADC3_IN6
        PA0/WKUP ------> ADC3_IN0 */
        HAL_GPIO_DeInit(GPIOF, ARDUINO_A4_Pin | ARDUINO_A5_Pin | ARDUINO_A1_Pin | ARDUINO_A2_Pin | ARDUINO_A3_Pin);
        HAL_GPIO_DeInit(ARDUINO_A0_GPIO_Port, ARDUINO_A0_Pin);
        /* ADC3 DMA DeInit */
        HAL_DMA_DeInit(adcHandle->DMA_Handle);
    }
}

void MX_TIM2_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 99;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 19;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;

    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;

    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }
}

void MX_DMA_Init(void) {
    __HAL_RCC_DMA2_CLK_ENABLE();
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}

void DMA2_Stream1_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_adc3);
}

/**
  * @brief  Conversion complete callback in non blocking mode
  * @param  AdcHandle : AdcHandle handle
  * @note   This example shows a simple way to report end of conversion, and
  *         you can add your own implementation.
  * @retval None
  */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *AdcHandle) {
    BSP_LED_Toggle(LED1);
    osSemaphoreRelease(xBinSem_ADCdoneHandle);
}
