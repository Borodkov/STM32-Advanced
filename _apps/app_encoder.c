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

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
typedef struct {
    uint32_t cur;
    uint32_t min;
    uint32_t max;
} Encoder_TypeDef;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim3;

osMessageQId osEncoderQueue;

static osSemaphoreId xBinSem_RotationDoneHandle = NULL;

/* Private function prototypes -----------------------------------------------*/
static void MX_TIM3_Init(void);

/**
    @brief
    @param
    @retval None
*/
static void task(void const *pvParameters) {
    LCD_UsrLog("vStartEncoderTask()\n");
    Encoder_TypeDef encoder;
    encoder.cur = 50;
    encoder.min = 0;
    encoder.max = 100;

    /* Infinite loop */
    for (;;) {
        osSemaphoreWait(xBinSem_RotationDoneHandle, osWaitForever); // w8 semaphore forever

        if (__HAL_TIM_IS_TIM_COUNTING_DOWN(&htim3)) {
            if (encoder.cur > encoder.min) { --encoder.cur; }
        } else {
            if (encoder.cur < encoder.max) { ++encoder.cur; }
        }

        osMessagePut(osEncoderQueue, encoder.cur, 10);
    }
}

/*----------------------------------------------------------------------------*/
void vStartEncoderTask() {
    MX_TIM3_Init();
    /* definition and creation of xBinSem_... */
    osSemaphoreDef(xBinSem_RotationDone);
    xBinSem_RotationDoneHandle = osSemaphoreCreate(osSemaphore(xBinSem_RotationDone), 1);
    /* take semaphore ... */
    osSemaphoreWait(xBinSem_RotationDoneHandle, 0);
    /*  Create the queue used by the two tasks to pass the incrementing number.
        Pass a pointer to the queue in the parameter structure. */
    osMessageQDef(osqueue, 10, uint32_t);
    osEncoderQueue = osMessageCreate(osMessageQ(osqueue), NULL);
    /* Create that task */
    osThreadDef(ENC,
                task,
                osPriorityNormal,
                0,
                2 * configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(ENC), NULL);
}

static void MX_TIM3_Init(void) {
    TIM_Encoder_InitTypeDef sConfig;
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 0xFFFF;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    sConfig.EncoderMode = TIM_ENCODERMODE_TI12;
    sConfig.IC1Polarity = TIM_ICPOLARITY_FALLING;
    sConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC1Prescaler = TIM_ICPSC_DIV4;
    sConfig.IC1Filter = 15;
    sConfig.IC2Polarity = TIM_ICPOLARITY_RISING;
    sConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;
    sConfig.IC2Prescaler = TIM_ICPSC_DIV4;
    sConfig.IC2Filter = 15;

    if (HAL_TIM_Encoder_Init(&htim3, &sConfig) != HAL_OK) {
        _Error_Handler(__FILE__, __LINE__);
    }

    __HAL_TIM_SET_COUNTER(&htim3, 0x8000);
    HAL_TIM_Encoder_Start_IT(&htim3, TIM_CHANNEL_ALL);
}

void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim_encoder) {
    GPIO_InitTypeDef GPIO_InitStruct;

    if (htim_encoder->Instance == TIM3) {
        __HAL_RCC_TIM3_CLK_ENABLE();
        __HAL_RCC_GPIOC_CLK_ENABLE();
        /** TIM3 GPIO Configuration
            PC7     ------> TIM3_CH2
            PC6     ------> TIM3_CH1
        */
        GPIO_InitStruct.Pin = GPIO_PIN_7 | GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_PULLUP;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(TIM3_IRQn, 10, 0);
        HAL_NVIC_EnableIRQ(TIM3_IRQn);
    }
}

void HAL_TIM_Encoder_MspDeInit(TIM_HandleTypeDef *htim_encoder) {
    if (htim_encoder->Instance == TIM3) {
        /* Peripheral clock disable */
        __HAL_RCC_TIM3_CLK_DISABLE();
        /** TIM3 GPIO Configuration
            PC7     ------> TIM3_CH2
            PC6     ------> TIM3_CH1
        */
        HAL_GPIO_DeInit(GPIOC, GPIO_PIN_7 | GPIO_PIN_6);
    }
}

void TIM3_IRQHandler(void) {
    __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_CC1 | TIM_IT_CC2);
    osSemaphoreRelease(xBinSem_RotationDoneHandle);
}
