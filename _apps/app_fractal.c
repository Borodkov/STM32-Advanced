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

#include "stm32746g_discovery.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_lcd.h"

#include "button.c"

/* Extern Variables ----------------------------------------------------------*/
extern osMessageQId osEncoderQueue;

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define SCREEN_SENSTIVITY 5

#define ITERATION       ((uint32_t)256)
#define FRACTAL_ZOOM    ((uint16_t)200)
#define REAL_CONSTANT   (0.285f)
#define IMG_CONSTANT    (0.01f)

#define FRACTAL_FRAME_BUFFER ((uint32_t)0xC0200000)

#if (__FPU_USED == 1)
#define SCORE_FPU_MODE    "FPU On"
#else
#define SCORE_FPU_MODE    "FPU Off"
#endif  /* __FPU_USED */

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static osSemaphoreId xBinSem_TouchDoneHandle = NULL;

DMA2D_HandleTypeDef    DMA2D_Handle;
uint32_t L8_CLUT[ITERATION];

uint8_t  text[50];

uint8_t SizeIndex = 6;
uint16_t SizeTable[][7] = {{478, 200}, {400, 200}, {300, 150}, {200, 100}, {160, 120}, {120, 80}, {100, 50}};

uint16_t XSize = 478;
uint16_t YSize = 200;

static TS_StateTypeDef TS_State;
__IO uint8_t TouchdOn = 0;
__IO uint8_t isplaying = 1;
__IO uint8_t playOneFrame = 0;
__IO uint32_t score_fpu = 0;
__IO uint32_t tickstart = 0;

/* Private function prototypes -----------------------------------------------*/
static void Generate_Julia_fpu(uint16_t size_x, uint16_t size_y, uint16_t offset_x, uint16_t offset_y, uint16_t zoom, uint8_t *buffer);
static void DMA2D_Init(uint32_t ImageWidth, uint32_t ImageHeight);
static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t ImageWidth, uint16_t ImageHeight);
static void DMA2D_CopyButton(uint32_t Button, uint32_t *pDst);
static void Touch_Handler(void const *pvParameters);
static void print_Size(void);

/**
    @brief
    @param
    @retval None
*/
static void task(void const *pvParameters) {
    uint32_t  zoomVal;
    uint32_t  r = 0, g = 0, b = 0;

    /* Color map generation */
    for (uint32_t  i = 0; i < ITERATION; i++) {
        /* Generate r, g and b values */
        r = (i * 8 * 256 / ITERATION) % 256;
        g = (i * 6 * 256 / ITERATION) % 256;
        b = (i * 4 * 256 / ITERATION) % 256;
        r = r << 16;
        g = g << 8;
        /* Store the 32-bit value */
        L8_CLUT[i] = 0xFF000000 | (r + g + b);
    }

    BSP_LCD_Clear(LCD_COLOR_WHITE);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    BSP_LCD_FillRect(1, 1, 480 - 2, 50 - 2);
    /*Title*/
    BSP_LCD_SetTextColor(LCD_COLOR_CYAN);
    BSP_LCD_SetFont(&Font16);
    BSP_LCD_DisplayStringAt(0, 8, (uint8_t *)"STM32F746 Fractal Benchmark", CENTER_MODE);
    BSP_LCD_DisplayStringAt(0, 24, (uint8_t *)SCORE_FPU_MODE, CENTER_MODE);
    BSP_LCD_SetFont(&Font12);
    /*Fractal Display area */
    BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
    BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
    BSP_LCD_FillRect(1, 50, 480 - 2, 200);
    BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
    BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
    /*image Size*/
    BSP_LCD_FillRect(1, 251, 160 - 1, 20);
    /*Zoom*/
    BSP_LCD_FillRect(161, 251, 160 - 2, 20);
    /*Calculation Time*/
    BSP_LCD_FillRect(320, 251, 160 - 1, 20);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(2, 255, (uint8_t *)"Size:", LEFT_MODE);
    BSP_LCD_DisplayStringAt(162, 255, (uint8_t *)"Zoom:", LEFT_MODE);
    BSP_LCD_DisplayStringAt(321, 255, (uint8_t *)"Duration:", LEFT_MODE);
    /* Init xsize and ysize used by fractal algo */
    XSize = SizeTable[SizeIndex][0];
    YSize = SizeTable[SizeIndex][1];
    /*print fractal image size*/
    print_Size();
    /*Copy Pause/Zoom in Off/Zoom out buttons*/
    DMA2D_CopyButton(PAUSE_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
    DMA2D_CopyButton(ZOOM_IN_OFF_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
    DMA2D_CopyButton(ZOOM_OUT_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
    DMA2D_Init(XSize, YSize);
    volatile osEvent event;

    /* Infinite loop */
    for (;;) {
        /* Get the message from the queue */
        event = osMessageGet(osEncoderQueue, osWaitForever);

        if (event.status == osEventMessage)
            if (event.value.v != zoomVal) {
                zoomVal = event.value.v;
            }

        if (isplaying || playOneFrame) {
            playOneFrame = 0;
            tickstart = HAL_GetTick();
            /* Start generating the fractal */
            Generate_Julia_fpu(XSize,
                               YSize,
                               XSize / 2,
                               YSize / 2,
                               zoomVal * 5,
                               (uint8_t *)FRACTAL_FRAME_BUFFER);
            /* Get elapsed time */
            score_fpu = (uint32_t)(HAL_GetTick() - tickstart);
            sprintf((char *)text, "Zoom: %03u [%]", zoomVal);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAt(161, 255, text, LEFT_MODE);
            sprintf((char *)text, "Duration: %03u [ms]", score_fpu);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            BSP_LCD_DisplayStringAt(321, 255, text, LEFT_MODE);
            /* Copy Result image to the display frame buffer*/
            DMA2D_CopyBuffer((uint32_t *)FRACTAL_FRAME_BUFFER/*buffer*/, (uint32_t *)LCD_FB_START_ADDRESS, XSize, YSize);
        }
    }
}

/*----------------------------------------------------------------------------*/
void vStartFractalTask() {
    LCD_UsrLog("vStartFractalTask()\n");
    /* definition and creation of xBinSem_... */
    osSemaphoreDef(xBinSem_ADCdone);
    xBinSem_TouchDoneHandle = osSemaphoreCreate(osSemaphore(xBinSem_ADCdone), 1);
    /* take semaphore ... */
    osSemaphoreWait(xBinSem_TouchDoneHandle, 0);
    /* Create that task */
    osThreadDef(FRACTAL,
                task,
                osPriorityRealtime,
                0,
                0x1000);
    osThreadCreate(osThread(FRACTAL), NULL);
    /* Create that task */
    osThreadDef(TOUCH,
                Touch_Handler,
                osPriorityBelowNormal,
                0,
                0x100);
    osThreadCreate(osThread(TOUCH), NULL);
}

static void Touch_Handler(void const *pvParameters) {
    uint32_t refreshScreen = 0;

    /* Infinite loop */
    for (;;) {
        osSemaphoreWait(xBinSem_TouchDoneHandle, osWaitForever); // w8 semaphore forever

        /*************************Pause/Play buttons *********************/
        if ((TS_State.touchX[0] + SCREEN_SENSTIVITY >= PLAY_PAUSE_BUTTON_XPOS) && \
                (TS_State.touchX[0] <= (PLAY_PAUSE_BUTTON_XPOS + BUTTON_WIDTH + SCREEN_SENSTIVITY)) && \
                (TS_State.touchY[0] + SCREEN_SENSTIVITY >= PLAY_PAUSE_BUTTON_YPOS) && \
                (TS_State.touchY[0] <= (PLAY_PAUSE_BUTTON_YPOS + BUTTON_HEIGHT + SCREEN_SENSTIVITY))) {
            isplaying = 1 - isplaying;

            if (isplaying == 0) {
                DMA2D_CopyButton(PLAY_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
            } else {
                DMA2D_CopyButton(PAUSE_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
            }

            DMA2D_Init(XSize, YSize);
        }
        /*************************Zoom In button *********************/
        else if ((TS_State.touchX[0] + SCREEN_SENSTIVITY >= ZOOM_IN_BUTTON_XPOS) && \
                 (TS_State.touchX[0] <= (ZOOM_IN_BUTTON_XPOS + BUTTON_WIDTH + SCREEN_SENSTIVITY)) && \
                 (TS_State.touchY[0] + SCREEN_SENSTIVITY >= ZOOM_IN_BUTTON_YPOS) && \
                 (TS_State.touchY[0] <= (ZOOM_IN_BUTTON_YPOS + BUTTON_HEIGHT + SCREEN_SENSTIVITY))) {
            if (SizeIndex > 0) {
                --SizeIndex;
                XSize = SizeTable[SizeIndex][0];
                YSize = SizeTable[SizeIndex][1];

                if (SizeIndex == 0) {
                    /*zoom in limit reached */
                    DMA2D_CopyButton(ZOOM_IN_OFF_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
                } else if (SizeIndex == 7) {
                    /* zoom out limit unreached display zoom out button */
                    DMA2D_CopyButton(ZOOM_OUT_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
                }

                refreshScreen = 1;
            }
        }
        /*************************Zoom Out button *********************/
        else if ((TS_State.touchX[0] + SCREEN_SENSTIVITY >= ZOOM_OUT_BUTTON_XPOS) && \
                 (TS_State.touchX[0] <= (ZOOM_OUT_BUTTON_XPOS + BUTTON_WIDTH + SCREEN_SENSTIVITY)) && \
                 (TS_State.touchY[0] + SCREEN_SENSTIVITY >= ZOOM_OUT_BUTTON_YPOS) && \
                 (TS_State.touchY[0] <= (ZOOM_OUT_BUTTON_YPOS + BUTTON_HEIGHT + SCREEN_SENSTIVITY))) {
            if (SizeIndex < 6) {
                ++SizeIndex;
                XSize = SizeTable[SizeIndex][0];
                YSize = SizeTable[SizeIndex][1];

                if (SizeIndex == 6) {
                    /* zoom out limit reached */
                    DMA2D_CopyButton(ZOOM_OUT_OFF_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
                } else if (SizeIndex == 1) {
                    /*zoom in limit unreached  Display zoom in button */
                    DMA2D_CopyButton(ZOOM_IN_BUTTON, (uint32_t *)LCD_FB_START_ADDRESS);
                }

                refreshScreen = 1;
            }
        }

        if (refreshScreen) {
            refreshScreen = 0;
            /*Clear Fractal Display area */
            BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
            BSP_LCD_SetTextColor(LCD_COLOR_BLACK);
            BSP_LCD_FillRect(1, 50, 480 - 2, 200);
            BSP_LCD_SetBackColor(LCD_COLOR_BLUE);
            BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
            print_Size();
            playOneFrame = 1;
            DMA2D_Init(XSize, YSize);
        }
    }
}

/**
    @brief  EXTI line detection callbacks
    @param  GPIO_Pin: Specifies the pins connected EXTI line
    @retval None
*/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {
    if (GPIO_Pin == TS_INT_PIN) {
        /* Get the IT status register value */
        BSP_TS_GetState(&TS_State);

        if (TS_State.touchDetected) {
            if (TouchdOn < 4) {
                TouchdOn++;
            } else { /*TouchReleased */
                TouchdOn = 0;
                osSemaphoreRelease(xBinSem_TouchDoneHandle);
            }
        }

        BSP_TS_ITClear();
    }
}

static void print_Size(void) {
    sprintf((char *)text, "Size: %u x %u", XSize, YSize);
    BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
    BSP_LCD_DisplayStringAt(2, 255, (uint8_t *)text, LEFT_MODE);
}

static void Generate_Julia_fpu(uint16_t size_x, uint16_t size_y, uint16_t offset_x, uint16_t offset_y, uint16_t zoom, uint8_t *buffer) {
    float       tmp1, tmp2;
    float       num_real, num_img;
    float       rayon;
    uint8_t       i;
    uint16_t      x, y;

    for (y = 0; y < size_y; y++) {
        for (x = 0; x < size_x; x++) {
            num_real = y - offset_y;
            num_real = num_real / zoom;
            num_img = x - offset_x;
            num_img = num_img / zoom;
            i = 0;
            rayon = 0;

            while ((i < ITERATION - 1) && (rayon < 4)) {
                tmp1 = num_real * num_real;
                tmp2 = num_img * num_img;
                num_img = 2 * num_real * num_img + IMG_CONSTANT;
                num_real = tmp1 - tmp2 + REAL_CONSTANT;
                rayon = tmp1 + tmp2;
                i++;
            }

            /* Store the value in the buffer */
            buffer[x + y * size_x] = i;
        }
    }
}

/**
    @brief  Initialize the DMA2D in memory to memory with PFC.
    @param  ImageWidth: image width
    @param  ImageHeight: image Height
    @retval None
*/
static void DMA2D_Init(uint32_t ImageWidth, uint32_t ImageHeight) {
    DMA2D_CLUTCfgTypeDef CLUTCfg;
    /* Init DMA2D */
    /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
    DMA2D_Handle.Init.Mode         = DMA2D_M2M_PFC;
    DMA2D_Handle.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
    DMA2D_Handle.Init.OutputOffset = BSP_LCD_GetXSize() - ImageWidth;
    /*##-2- DMA2D Callbacks Configuration ######################################*/
    DMA2D_Handle.XferCpltCallback  = NULL;
    /*##-3- Foreground Configuration ###########################################*/
    DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    DMA2D_Handle.LayerCfg[1].InputAlpha = 0xFF;
    DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_L8;
    DMA2D_Handle.LayerCfg[1].InputOffset = 0;
    DMA2D_Handle.Instance          = DMA2D;
    /*##-4- DMA2D Initialization     ###########################################*/
    HAL_DMA2D_Init(&DMA2D_Handle);
    HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1);
    /* Load DMA2D Foreground CLUT */
    CLUTCfg.CLUTColorMode = DMA2D_CCM_ARGB8888;
    CLUTCfg.pCLUT = (uint32_t *)L8_CLUT;
    CLUTCfg.Size = 255;
    HAL_DMA2D_CLUTLoad(&DMA2D_Handle, CLUTCfg, 1);
    HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 100);
}

/**
    @brief  Copy the Decoded image to the display Frame buffer.
    @param  pSrc: Pointer to source buffer
    @param  pDst: Pointer to destination buffer
    @param  ImageWidth: image width
    @param  ImageHeight: image Height
    @retval None
*/
static void DMA2D_CopyBuffer(uint32_t *pSrc, uint32_t *pDst, uint16_t ImageWidth, uint16_t ImageHeight) {
    uint32_t xPos, yPos, destination;
    /*##-1- calculate the destination transfer address  ############*/
    xPos = (BSP_LCD_GetXSize() - ImageWidth) / 2;
    yPos = 50;
    destination = (uint32_t)pDst + (yPos * BSP_LCD_GetXSize() + xPos) * 4;
    HAL_DMA2D_Start(&DMA2D_Handle, (uint32_t)pSrc, destination, ImageWidth, ImageHeight);
    HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 100);
}

static void DMA2D_CopyButton(uint32_t Button, uint32_t *pDst) {
    uint32_t xPos = 0;
    uint32_t yPos = 0;
    uint32_t destination, buttonWidth;
    uint32_t *pSrc = NULL;

    if (PLAY_BUTTON == Button) {
        xPos = PLAY_PAUSE_BUTTON_XPOS;
        yPos = PLAY_PAUSE_BUTTON_YPOS;
        buttonWidth = BUTTON_WIDTH;
        pSrc = (uint32_t *)Play_Button;
    } else if (PAUSE_BUTTON == Button) {
        xPos = PLAY_PAUSE_BUTTON_XPOS;
        yPos = PLAY_PAUSE_BUTTON_YPOS;
        buttonWidth = BUTTON_WIDTH;
        pSrc = (uint32_t *)Pause_Button;
    } else if (ZOOM_IN_BUTTON == Button) {
        xPos = ZOOM_IN_BUTTON_XPOS;
        yPos = ZOOM_IN_BUTTON_YPOS;
        buttonWidth = BUTTON_WIDTH;
        pSrc = (uint32_t *)ZOOM_IN_Button;
    } else if (ZOOM_OUT_BUTTON == Button) {
        xPos = ZOOM_OUT_BUTTON_XPOS;
        yPos = ZOOM_OUT_BUTTON_YPOS;
        buttonWidth = BUTTON_WIDTH;
        pSrc = (uint32_t *)ZOOM_Out_Button;
    } else if (ZOOM_IN_OFF_BUTTON == Button) {
        xPos = ZOOM_IN_BUTTON_XPOS;
        yPos = ZOOM_IN_BUTTON_YPOS;
        buttonWidth = BUTTON_WIDTH;
        pSrc = (uint32_t *)ZOOM_IN_Off_Button;
    } else if (ZOOM_OUT_OFF_BUTTON == Button) {
        xPos = ZOOM_OUT_BUTTON_XPOS;
        yPos = ZOOM_OUT_BUTTON_YPOS;
        buttonWidth = BUTTON_WIDTH;
        pSrc = (uint32_t *)ZOOM_Out_Off_Button;
    }

    /* Init DMA2D */
    /*##-1- Configure the DMA2D Mode, Color Mode and output offset #############*/
    DMA2D_Handle.Init.Mode         = DMA2D_M2M_PFC;
    DMA2D_Handle.Init.ColorMode    = DMA2D_OUTPUT_ARGB8888;
    DMA2D_Handle.Init.OutputOffset = BSP_LCD_GetXSize() - buttonWidth;
    /*##-2- DMA2D Callbacks Configuration ######################################*/
    DMA2D_Handle.XferCpltCallback  = NULL;
    /*##-3- Foreground Configuration ###########################################*/
    DMA2D_Handle.LayerCfg[1].AlphaMode = DMA2D_REPLACE_ALPHA;
    DMA2D_Handle.LayerCfg[1].InputAlpha = 0xFF;
    DMA2D_Handle.LayerCfg[1].InputColorMode = DMA2D_INPUT_ARGB8888;
    DMA2D_Handle.LayerCfg[1].InputOffset = 0;
    DMA2D_Handle.Instance          = DMA2D;
    /*##-4- DMA2D Initialization     ###########################################*/
    HAL_DMA2D_Init(&DMA2D_Handle);
    HAL_DMA2D_ConfigLayer(&DMA2D_Handle, 1);
    destination = (uint32_t)pDst + (yPos * BSP_LCD_GetXSize() + xPos) * 4;
    HAL_DMA2D_Start(&DMA2D_Handle, (uint32_t)pSrc, destination, BUTTON_WIDTH, BUTTON_HEIGHT);
    HAL_DMA2D_PollForTransfer(&DMA2D_Handle, 100);
}
