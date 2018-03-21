/**
  ******************************************************************************
    File Name          : main.c
    Description        : Main program body
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h"
#include "usb_device.h"
#include "usbd_cdc_if.h"

#include "scope.h"
#ifdef USE_STEMWIN
#include "GUI.h"
#include "WM.h"
#include "GRAPH.h"
#endif

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc3;
DMA_HandleTypeDef hdma_adc3;

DMA2D_HandleTypeDef hdma2d;

LTDC_HandleTypeDef hltdc;

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim8;

SDRAM_HandleTypeDef hsdram1;

uint32_t value = 0;

short aData0[NUM_SAMPLES * DUAL_BUFFER]; // 2 is dual buffer reservation
short sendData0[NUM_SAMPLES];
uint32_t samplesRemaining = 0;
OSC_TypeDef oscConfig = {
    1000, 1000, DEFAULT_TRIGGER_LEVEL, 0, TRIGGER_AUTO
};
uint8_t AdcConversionDone = 0;
uint8_t flushUSB = 0;

#ifdef USE_FIR
#define NUM_SAMPLES_FFT 1000

q15_t iirCoeff[12] = {
    7516 , 0, 407, 7516, -20843, 11516,
    7516, 0, 407, 7516, -20843, 11516
};

// q15_t iirCoeff[12] = {
//   7516 / 2, 0, 407 / 2, 7516 / 2, -28843 / 2, 11516 / 2,
//   7516 / 2, 0, 407 / 2, 7516 / 2, -28843 / 2, 11516 / 2
// };

// q15_t iirCoeff[12] = {
//   1000, 0, 0, 0, 0, 0,
//   1000, 0, 0, 0, 0, 0
// };

q15_t left[NUM_SAMPLES_FFT];
q15_t pIIRStateLeft[4 * 2];
arm_biquad_casd_df1_inst_q15 Sleft, Sright;

#endif

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_ADC3_Init(void);
static void MX_DMA2D_Init(void);
static void MX_FMC_Init(void);
static void MX_LTDC_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM8_Init(void);

void LCD_Config(void);
void SDRAM_Config(void);

int main(void) {
    /* MCU Configuration----------------------------------------------------------*/
    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
    HAL_Init();
    /* Configure the system clock */
    SystemClock_Config();
    /* Initialize all configured peripherals */
    MX_GPIO_Init();
    MX_DMA_Init();
    MX_ADC3_Init();
    MX_DMA2D_Init();
    MX_FMC_Init();
    MX_LTDC_Init();
    MX_TIM2_Init();
    MX_TIM8_Init();
    MX_USB_DEVICE_Init();
    /* Enable tim8 oc generator */
    HAL_TIM_OC_Start(&htim8, TIM_CHANNEL_4);
#ifdef USE_STEMWIN
    ScopeLCD_Init();
#endif
    /*start ADC sampling into buffer*/
    HAL_ADC_Start_DMA(&hadc3, (uint32_t *)aData0, (oscConfig.numberOfSamples * DUAL_BUFFER * LONG_BUFFER));
    /*start TIM which triggers ADC conversions*/
    HAL_TIM_OC_Start(&htim2, TIM_CHANNEL_2);
#ifdef USE_FIR
    arm_biquad_cascade_df1_init_q15(&Sleft, 2, iirCoeff, pIIRStateLeft, 0);
#endif

    /* Infinite loop */
    while (1) {
        uint32_t i;
        USBD_CDC_HandleTypeDef   *hcdc;

        /*check if tha ADC conversion is done*/
        if (AdcConversionDone) {
            uint32_t triggerSample;
            int16_t *bufferstart;
            uint32_t temporary;
            /*buffer start*/
            temporary = ((uint32_t)AdcConversionDone - 0x1);
            temporary = (temporary * (oscConfig.numberOfSamples * LONG_BUFFER));
            bufferstart = &aData0[temporary];
            // do a simple trigger on edge
            triggerSample = ScopeProcess(bufferstart);

            /*check if trigger is present*/
            if (triggerSample != SCOPE_INV_TRIGGER) {
                /*mark trigger positiopn*/
                i = triggerSample + 1;

                // if USB ready, start transmitting
                if (hUsbDeviceFS.dev_state == USBD_STATE_CONFIGURED && (hcdc = hUsbDeviceFS.pClassData) != 0 && hcdc->TxState == 0) {
                    // copy now the ADC samples into USB buffer
                    for (i = 0; i < (oscConfig.numberOfSamples /*- triggerSample*/); i++) {
                        sendData0[i] = bufferstart[triggerSample + i];
                    }

                    CDC_Transmit_FS((uint8_t *)sendData0, oscConfig.numberOfSamples * NUM_BYTES_PER_SAMPLE);

                    /*wait untill data was sent*/
                    while (hcdc->TxState != 0);

                    // show the same data on screen as well
                    ScopeLCD_Show(sendData0);
                    /*prepare for new adc conversion*/
                    AdcConversionDone = 0;
                }
            }
        }
    }
}

/* System Clock Configuration */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct;
    HAL_PWREx_ActivateOverDrive();
    __PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 25;
    RCC_OscInitStruct.PLL.PLLN = 400;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                  | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6);
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_LTDC | RCC_PERIPHCLK_CLK48;
    PeriphClkInitStruct.PLLSAI.PLLSAIN = 192;
    PeriphClkInitStruct.PLLSAI.PLLSAIR = 5;
    PeriphClkInitStruct.PLLSAI.PLLSAIQ = 2;
    PeriphClkInitStruct.PLLSAI.PLLSAIP = RCC_PLLSAIP_DIV4;
    PeriphClkInitStruct.PLLSAIDivQ = 1;
    PeriphClkInitStruct.PLLSAIDivR = RCC_PLLSAIDIVR_4;
    PeriphClkInitStruct.Clk48ClockSelection = RCC_CLK48SOURCE_PLLSAIP;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct);
}

/* ADC3 init function */
void MX_ADC3_Init(void) {
    ADC_ChannelConfTypeDef sConfig;
    /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
    */
    hadc3.Instance = ADC3;
    hadc3.Init.ClockPrescaler = ADC_CLOCKPRESCALER_PCLK_DIV4;
    hadc3.Init.Resolution = ADC_RESOLUTION12b;
    hadc3.Init.ScanConvMode = DISABLE;
    hadc3.Init.ContinuousConvMode = DISABLE;
    hadc3.Init.DiscontinuousConvMode = DISABLE;
    hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISINGFALLING;
    hadc3.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_CC2;
    hadc3.Init.DataAlign = ADC_DATAALIGN_RIGHT;
    hadc3.Init.NbrOfConversion = 1;
    hadc3.Init.DMAContinuousRequests = ENABLE;
    hadc3.Init.EOCSelection = EOC_SINGLE_CONV;
    HAL_ADC_Init(&hadc3);
    /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
    */
    sConfig.Channel = ADC_CHANNEL_0;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
    HAL_ADC_ConfigChannel(&hadc3, &sConfig);
}

/* DMA2D init function */
void MX_DMA2D_Init(void) {
    hdma2d.Instance = DMA2D;
    hdma2d.Init.Mode = DMA2D_M2M;
    hdma2d.Init.ColorMode = DMA2D_ARGB8888;
    hdma2d.Init.OutputOffset = 0;
    hdma2d.LayerCfg[1].InputOffset = 0;
    hdma2d.LayerCfg[1].InputColorMode = CM_ARGB8888;
    hdma2d.LayerCfg[1].AlphaMode = DMA2D_NO_MODIF_ALPHA;
    hdma2d.LayerCfg[1].InputAlpha = 0;
    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, 1);
}

/* LTDC init function */
void MX_LTDC_Init(void) {
    hltdc.Instance = LTDC;
    hltdc.Init.HSPolarity = LTDC_HSPOLARITY_AL;
    hltdc.Init.VSPolarity = LTDC_VSPOLARITY_AL;
    hltdc.Init.DEPolarity = LTDC_DEPOLARITY_AL;
    hltdc.Init.PCPolarity = LTDC_PCPOLARITY_IPC;
    hltdc.Init.HorizontalSync = 7;
    hltdc.Init.VerticalSync = 3;
    hltdc.Init.AccumulatedHBP = 14;
    hltdc.Init.AccumulatedVBP = 5;
    hltdc.Init.AccumulatedActiveW = 654;
    hltdc.Init.AccumulatedActiveH = 485;
    hltdc.Init.TotalWidth = 660;
    hltdc.Init.TotalHeigh = 487;
    hltdc.Init.Backcolor.Blue = 0;
    hltdc.Init.Backcolor.Green = 0;
    hltdc.Init.Backcolor.Red = 0;
    HAL_LTDC_Init(&hltdc);
}

/* TIM2 init function */
void MX_TIM2_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_OC_InitTypeDef sConfigOC;
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 5000;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig);
    HAL_TIM_OC_Init(&htim2);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);
    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);
}

/* TIM8 init function */
void MX_TIM8_Init(void) {
    TIM_ClockConfigTypeDef sClockSourceConfig;
    TIM_MasterConfigTypeDef sMasterConfig;
    TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig;
    TIM_OC_InitTypeDef sConfigOC;
    htim8.Instance = TIM8;
    htim8.Init.Prescaler = 9;
    htim8.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim8.Init.Period = 9999;
    htim8.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim8.Init.RepetitionCounter = 0;
    HAL_TIM_Base_Init(&htim8);
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    HAL_TIM_ConfigClockSource(&htim8, &sClockSourceConfig);
    HAL_TIM_OC_Init(&htim8);
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterOutputTrigger2 = TIM_TRGO2_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim8, &sMasterConfig);
    sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
    sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
    sBreakDeadTimeConfig.DeadTime = 0;
    sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
    sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
    sBreakDeadTimeConfig.BreakFilter = 0;
    sBreakDeadTimeConfig.Break2State = TIM_BREAK2_DISABLE;
    sBreakDeadTimeConfig.Break2Polarity = TIM_BREAK2POLARITY_HIGH;
    sBreakDeadTimeConfig.Break2Filter = 0;
    sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
    HAL_TIMEx_ConfigBreakDeadTime(&htim8, &sBreakDeadTimeConfig);
    sConfigOC.OCMode = TIM_OCMODE_TOGGLE;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
    HAL_TIM_OC_ConfigChannel(&htim8, &sConfigOC, TIM_CHANNEL_4);
}

/**
    Enable DMA controller clock
*/
void MX_DMA_Init(void) {
    /* DMA controller clock enable */
    __HAL_RCC_DMA2_CLK_ENABLE();
    /* DMA interrupt init */
    HAL_NVIC_SetPriority(DMA2_Stream1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream1_IRQn);
}
/* FMC initialization function */
void MX_FMC_Init(void) {
    FMC_SDRAM_TimingTypeDef SdramTiming;
    /** Perform the SDRAM1 memory initialization sequence
    */
    hsdram1.Instance = FMC_SDRAM_DEVICE;
    /* hsdram1.Init */
    hsdram1.Init.SDBank = FMC_SDRAM_BANK1;
    hsdram1.Init.ColumnBitsNumber = FMC_SDRAM_COLUMN_BITS_NUM_8;
    hsdram1.Init.RowBitsNumber = FMC_SDRAM_ROW_BITS_NUM_12;
    hsdram1.Init.MemoryDataWidth = FMC_SDRAM_MEM_BUS_WIDTH_16;
    hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram1.Init.CASLatency = FMC_SDRAM_CAS_LATENCY_2;
    hsdram1.Init.WriteProtection = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram1.Init.SDClockPeriod = FMC_SDRAM_CLOCK_PERIOD_2;
    hsdram1.Init.ReadBurst = FMC_SDRAM_RBURST_ENABLE;
    hsdram1.Init.ReadPipeDelay = FMC_SDRAM_RPIPE_DELAY_2;
    /* SdramTiming */
    SdramTiming.LoadToActiveDelay = 2;
    SdramTiming.ExitSelfRefreshDelay = 7;
    SdramTiming.SelfRefreshTime = 4;
    SdramTiming.RowCycleDelay = 8;
    SdramTiming.WriteRecoveryTime = 4;
    SdramTiming.RPDelay = 2;
    SdramTiming.RCDDelay = 2;
    HAL_SDRAM_Init(&hsdram1, &SdramTiming);
}

/** Configure pins as
          Analog
          Input
          Output
          EVENT_OUT
          EXTI
*/
void MX_GPIO_Init(void) {
    GPIO_InitTypeDef GPIO_InitStruct;
    /* GPIO Ports Clock Enable */
    __GPIOE_CLK_ENABLE();
    __GPIOG_CLK_ENABLE();
    __GPIOJ_CLK_ENABLE();
    __GPIOD_CLK_ENABLE();
    __GPIOA_CLK_ENABLE();
    __GPIOK_CLK_ENABLE();
    __GPIOI_CLK_ENABLE();
    __GPIOF_CLK_ENABLE();
    __GPIOH_CLK_ENABLE();
    __GPIOC_CLK_ENABLE();
    /*Configure GPIO pin : PK3 */
    GPIO_InitStruct.Pin = GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOK, &GPIO_InitStruct);
    /*Configure GPIO pin : PI12 */
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
    HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);
    /*Configure GPIO pin : PF10 */
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
    AdcConversionDone = 1;
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    AdcConversionDone = 2;
}

#ifdef USE_FULL_ASSERT
/**
     @brief Reports the name of the source file and the source line number
     where the assert_param error has occurred.
     @param file: pointer to the source file name
     @param line: assert_param error line source number
     @retval None
*/
void assert_failed(uint8_t *file, uint32_t line) {
    /* USER CODE BEGIN 6 */
    /*  User can add his own implementation to report the file name and line number,
        ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
    /* USER CODE END 6 */
}

#endif
