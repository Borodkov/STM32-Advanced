// scope.c

#include "stm32f7xx_hal.h"
#include "osc.h"

#ifdef USE_STEMWIN
#include "GUI.h"
#include "WM.h"
#include "GRAPH.h"
#endif

uint32_t triggerSample;
uint32_t lastTrigger;
uint32_t triggered = 0;

extern ADC_HandleTypeDef hadc3;
extern DMA_HandleTypeDef hdma_adc3;

#ifdef USE_STEMWIN
WM_HWIN hGraph;
GRAPH_DATA_Handle  hData1, hData2;
GRAPH_SCALE_Handle hScale;
#endif


/*Find trigger function
*arg uint32_t triggerEdge - specify the trigger orientation, 0 for rising edge, >0 for falling edge
*arg short triggerLevel - trigger level setup
*arg short* pData - pointer to 16bit data
*arg uint32_t numberOfSamples - size of pData array
*return uint32_t - the specific sample where is fullfill trigger condition
*/
uint32_t ScopeFindTrigger(uint32_t triggerEdge, short triggerLevel, short* pData, uint32_t numberOfSamples) {
    int prevCondition;
    uint32_t triggered, triggerSample, i;
    triggered = 0;
    triggerSample = SCOPE_INV_TRIGGER;
    i = 1;

    /*check if we searching rising or falling edge*/
    if (triggerEdge == 0) {
        /*Rising Edge*/
        /*Check if the first sample is above trigger level*/
        prevCondition = pData[0] >= triggerLevel;

        /*loop where we search in array for trigger condition
          *(i < numberOfSamples) - check if we still are in array space
          * AND
          *(triggered ==0) - check if trigger was found
          */
        while ((i < numberOfSamples - 2) && (triggered == 0)) {
            /*checking if current sample is above trigger condition*/
            int condition = (pData[i] >= triggerLevel)  && (pData[i + 1] >= triggerLevel) && (pData[i + 2] >= triggerLevel);

            /*check it trigger occure
            *!prevCondition - the previous sample must be under trigger level
            *AND
            *condition - and this sample must be above trigger level to reach triggering condition
            */
            if (!prevCondition && condition) {
                /*mark which sample is the trigger condition*/
                triggerSample = i - 1;
                /*inform loop that we found trigger*/
                triggered = 1;
            }

            /*mowe current sample to prefious variable for next loop run*/
            prevCondition = condition;
            /*increment loop counter*/
            i++;
        }
    } else {
        /*Faling Edge*/
        /*Check if the first sample is above trigger level*/
        prevCondition = pData[0] >= triggerLevel;

        /*loop where we search in array for trigger condition
        *(i < numberOfSamples) - check if we still are in array space
        * AND
        *(triggered ==0) - check if trigger was found
        */
        while (i < (numberOfSamples - 4) && triggered == 0) {
            /*checking if current sample is above trigger condition*/
            int condition = (pData[i] >= triggerLevel) && (pData[i + 1] >= triggerLevel) && (pData[i + 2] >= triggerLevel) && (pData[i + 3] >= triggerLevel) && (pData[i + 4] >= triggerLevel);

            /*check it trigger occure
            *prevCondition - the previous sample must be above trigger level
            *AND
            *!condition - and this sample must be under trigger level to reach triggering condition
            */
            if (prevCondition && !condition) {
                /*mark which sample is the trigger condition*/
                triggerSample = i - 1;
                /*inform loop that we found trigger*/
                triggered = 1;
            }

            /*mowe current sample to prefious variable for next loop run*/
            prevCondition = condition;
            /*increment loop counter*/
            i++;
        }
    }

    /*return which sample in pData array is trigger*/
    return triggerSample;
}

/*
* ScopeProcess function
* arg short * pData - array with sampled data
* return uint32_t sample position in pData array which is the trigger
*/
uint32_t ScopeProcess(short * pData) {
    /*find possition of trigger in pData array*/
    triggerSample = ScopeFindTrigger(oscConfig.triggerEdge, oscConfig.triggerLevel, pData, oscConfig.numberOfSamples);

    /*check if the trigger exists*/
    if (triggerSample != SCOPE_INV_TRIGGER) {
        /*trigger the cope*/
        triggered = 1;
        /*mark trigger time*/
        lastTrigger = HAL_GetTick();
    }
    /*trigger not exists*/
    else {
        /*not trigger scope*/
        triggered = 0;
    }

    /*Test if is possible to trigger application again in auto mode
    *oscConfig.triggerMode == TRIGGER_AUTO - check if application is in auto trigger mode
    *AND
    *(HAL_GetTick() - lastTrigger) > TRIGGER_TIMEOUT - check if application can trigger again
    */
    if (oscConfig.triggerMode == TRIGGER_AUTO && (HAL_GetTick() - lastTrigger) > TRIGGER_TIMEOUT) {
        /*auto mode application didn't wait on trigger*/
        triggerSample = 0;
        /*trigger*/
        triggered = 1;
        /*mark when was this trigger*/
        lastTrigger = HAL_GetTick();
    }

    /*check if application is in single trigger mode*/
    if (oscConfig.triggerMode == TRIGGER_SINGLE && triggered == 1) {
        /*disable trigger for next run*/
        oscConfig.triggerMode = TRIGGER_DISABLED;
    }

    /*if the number of sampes was changed*/
    if (oscConfig.numberOfSamples != oscConfig.newNumberOfSamples) {
        /*store information about new requested sample count*/
        oscConfig.numberOfSamples = oscConfig.newNumberOfSamples;

        /*check if application can handle nuber of samples*/
        if (oscConfig.numberOfSamples > NUM_SAMPLES) { oscConfig.numberOfSamples = NUM_SAMPLES; }

        /*check if number of samples is not low*/
        if (oscConfig.numberOfSamples < 16) { oscConfig.numberOfSamples = 16; }

        /*store actual number of samples after length corrections*/
        oscConfig.newNumberOfSamples = oscConfig.numberOfSamples;
        /*stop ADC*/
        HAL_ADC_Stop_DMA(&hadc3);
        /*start ADC with new pData array width*/
        HAL_ADC_Start_DMA(&hadc3, (uint32_t*)pData, (oscConfig.numberOfSamples * DUAL_BUFFER * LONG_BUFFER));
    }

    /*return information about trigger*/
    return triggerSample;
}

/*scale the pDataIn array
*arg short * pDataIn - input array for scale
*arg short * pDataOut - output scalled array
*arg uint32_t inMax - input maximum value
*arg uint32_t outMax - new output maximum value
*arg uint32_t numSamples - array widtd
*/
void ScopeScale(short * pDataIn, uint16_t * pDataOut, int32_t inMax, int32_t outMax, uint32_t numSamples) {
    uint32_t i;

    /*loop for array*/
    for (i = 0; i < numSamples; i++) {
        /*scale values*/
        pDataOut[i] = ((((int32_t)pDataIn[i] + inMax / 2) * outMax) / inMax);
    }
}


/*
* Show the pData array on LCD
*/
void ScopeLCD_Show(short * pData1, short * pData2) {
#ifdef USE_STEMWIN
    uint32_t numToShow;
    // check that there is enough samples to show
    //if((oscConfig.numberOfSamples*2) - triggerSample > LCD_GetXSize()-GRAPH_BORDER_L)
    numToShow = MIN(LCD_GetXSize() - GRAPH_BORDER_L, NUM_SAMPLES);
    //else
    /*calculate how many samples application can show*/
    //numToShow = (oscConfig.numberOfSamples*2) - triggerSample;
    /*scale pData buffer*/
    ScopeScale(pData1, pData1, 4096,  MIN(LCD_GetYSize(), NUM_SAMPLES), numToShow);   // scale the data amplitude for show
    ScopeScale(pData2, pData2, 4096,  MIN(LCD_GetYSize(), NUM_SAMPLES), numToShow);   // scale the data amplitude for show
    /*Plot Y axis*/
    hData1  = GRAPH_DATA_YT_Create(ST_BLUE, numToShow, pData1, numToShow);
    hData2  = GRAPH_DATA_YT_Create(ST_PINK, numToShow, pData2, numToShow);
    /*Setup allign of Y axis*/
    GRAPH_DATA_YT_SetAlign(hData1, GRAPH_ALIGN_LEFT);
    GRAPH_DATA_YT_SetAlign(hData2, GRAPH_ALIGN_LEFT);
    /*put data to STemWin component*/
    GRAPH_AttachData(hGraph, hData1);
    GRAPH_AttachData(hGraph, hData2);
    /*plot data*/
    GUI_Exec(); // show the graph
    /*data was displayed not we nedd to clead graph component for next run*/
    GRAPH_DetachData(hGraph, hData1);
    GRAPH_DetachData(hGraph, hData2);
    /*data was displayed not we nedd to clead Y axe component for next run*/
    GRAPH_DATA_YT_Delete(hData1);
    GRAPH_DATA_YT_Delete(hData2);
#endif
}
/*
* Initialize LCD TFT display
*/
void ScopeLCD_Init(void) {
#ifdef  USE_STEMWIN
    /* Enable the CRC Module */
    __CRC_CLK_ENABLE();
    /*Init STemWin*/
    GUI_Init();
    /*use more buffers*/
    WM_MULTIBUF_Enable(1);
    /*Create graph component*/
    hGraph = GRAPH_CreateEx(0, 0, LCD_GetXSize(), LCD_GetYSize(), WM_HBKWIN, WM_CF_SHOW, 0, GUI_ID_GRAPH0);
    /*set graph borders*/
    GRAPH_SetBorder(hGraph, GRAPH_BORDER_L, 0, 0, 0);
    GRAPH_SetColor(hGraph, GUI_WHITE, GRAPH_CI_BK);
    /*create scale component for graph*/
    hScale = GRAPH_SCALE_Create(15, GUI_TA_LEFT, GRAPH_SCALE_CF_VERTICAL, 20);
    /*attach scale compnent to graph component*/
    GRAPH_AttachScale(hGraph, hScale);
    /*Set graph scaling*/
    GRAPH_SCALE_SetNumDecs(hScale, 2);
    // scale to full 1 resolution
    GRAPH_SCALE_SetFactor(hScale, (float)1 / (float)LCD_GetYSize() * (float)2);
    GRAPH_SCALE_SetOff(hScale, LCD_GetYSize() / 2);
#endif
}
