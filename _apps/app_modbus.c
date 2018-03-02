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
#include "cmsis_os.h"
#include "lcd_log.h"

/* ------------------------ Platform includes --------------------------------*/
#include "port.h"

/* ------------------------ Modbus includes ----------------------------------*/
#include "mb.h"

/* External variables --------------------------------------------------------*/
extern RTC_HandleTypeDef hrtc;
extern RTC_TimeTypeDef time;
extern RTC_DateTypeDef date;

/* Private define ------------------------------------------------------------*/
#define REG_INPUT_START   1000
#define REG_INPUT_NREGS   4
#define REG_HOLDING_START 1
#define REG_HOLDING_NREGS 40

/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osThreadId MODBUSHandle;
static USHORT   usRegInputBuf[REG_INPUT_NREGS];
static USHORT   usRegHoldingStart = REG_HOLDING_START;
static USHORT   usRegHoldingBuf[REG_HOLDING_NREGS];
/* Private function prototypes -----------------------------------------------*/

/**
  * @brief
  * @param
  * @retval None
  */
static void task(void const *pvParameters) {
  eMBErrorCode eStatus;
  portTickType xLastWakeTime;
  /* Select either ASCII or RTU Mode. */
  eStatus = eMBInit(MB_RTU, 0x01, 0, 115200, MB_PAR_NONE);
  /* Enable the Modbus Protocol Stack. */
  eStatus = eMBEnable();

  /* Infinite loop */
  for (;;) {
    /* Call the main polling loop of the Modbus protocol stack. */
    (void)eMBPoll();
    /* Application specific actions. Count the number of poll cycles. */
    usRegInputBuf[0]++;
    /* Hold the current FreeRTOS ticks. */
    xLastWakeTime = xTaskGetTickCount();
    usRegInputBuf[1] = (unsigned portSHORT)(xLastWakeTime >> 16UL);
    usRegInputBuf[2] = (unsigned portSHORT)(xLastWakeTime & 0xFFFFUL);
    /* The constant value. */
    usRegInputBuf[3] = 33;
    osDelay(20);
  }
}

/*----------------------------------------------------------------------------*/
void vStartModBusTask() {
  LCD_UsrLog("vStartModBusTask()\n");
  /* Create that task */
  osThreadDef(MODBUS,
              task,
              osPriorityHigh,
              0,
              0x400);
  MODBUSHandle = osThreadCreate(osThread(MODBUS), NULL);
}

eMBErrorCode
eMBRegHoldingCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs, eMBRegisterMode eMode) {
  eMBErrorCode    eStatus = MB_ENOERR;
  int             iRegIndex;

  if ((usAddress >= REG_HOLDING_START) &&
      (usAddress + usNRegs <= REG_HOLDING_START + REG_HOLDING_NREGS)) {
    iRegIndex = (int)(usAddress - usRegHoldingStart);

    switch (eMode) {
    /* Pass current register values to the protocol stack. */
    case MB_REG_READ:

      // copy string with RTC and Temperature to the holding buf
//     memcpy(usRegHoldingBuf, sDateTemp, REG_HOLDING_NREGS);
      while (usNRegs > 0) {
//       *pucRegBuffer++ = ( unsigned char )( sDateTemp[iRegIndex++]);
//       *pucRegBuffer++ = ( unsigned char )( sDateTemp[iRegIndex++]);
        *pucRegBuffer++ = (unsigned char)(usRegHoldingBuf[iRegIndex] >> 8);
        *pucRegBuffer++ = (unsigned char)(usRegHoldingBuf[iRegIndex] & 0xFF);
        iRegIndex++;
        usNRegs--;
      }

      break;

    /* Update current register values with new values from the protocol stack. */
    case MB_REG_WRITE:
      while (usNRegs > 0) {
        usRegHoldingBuf[iRegIndex] = *pucRegBuffer++ << 8;
        usRegHoldingBuf[iRegIndex] |= *pucRegBuffer++;
        iRegIndex++;
        usNRegs--;
      }

      // update date & time
      iRegIndex = (int)(usAddress - usRegHoldingStart);
      date.Year    = (uint8_t)(usRegHoldingBuf[iRegIndex++]);
      date.Month   = (uint8_t)(usRegHoldingBuf[iRegIndex++]);
      date.Date    = (uint8_t)(usRegHoldingBuf[iRegIndex++]);
      date.WeekDay = (uint8_t)(usRegHoldingBuf[iRegIndex++]);
      time.Hours   = (uint8_t)(usRegHoldingBuf[iRegIndex++]);
      time.Minutes = (uint8_t)(usRegHoldingBuf[iRegIndex++]);
      time.Seconds = (uint8_t)(usRegHoldingBuf[iRegIndex++]);
      HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
      HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
    }
  } else {
    eStatus = MB_ENOREG;
  }

  return eStatus;
}

eMBErrorCode
eMBRegInputCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNRegs) {
  return MB_ENOREG;
}

eMBErrorCode
eMBRegCoilsCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNCoils, eMBRegisterMode eMode) {
  return MB_ENOREG;
}

eMBErrorCode
eMBRegDiscreteCB(UCHAR * pucRegBuffer, USHORT usAddress, USHORT usNDiscrete) {
  return MB_ENOREG;
}
