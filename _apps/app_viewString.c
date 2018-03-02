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
//#include "usbd_cdc_if.h"
#include "lwip/api.h"

/* Extern function prototypes ------------------------------------------------*/
/* Registers a set of example commands that can be used in the command console*/
extern void vRegisterSampleCLICommands(void);

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern RTC_TimeTypeDef time;
extern RTC_DateTypeDef date;

extern osMutexId xUSBTxMutex;
//extern osSemaphoreId xBinSem_SDHandle;
extern osSemaphoreId xBinSem_UDPSysLogHandle;
extern struct netconn *TelNetConn;

static osSemaphoreId xBinSem_viewStringHandle = NULL;
static char strToView[100];
const char *weekDays[] = {"TMP", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};

/* Private function prototypes -----------------------------------------------*/

/**
  * @brief
  * @param
  * @retval None
  */
static void task(void const *pvParameters) {
  for (;;) {
    osSemaphoreWait(xBinSem_viewStringHandle, osWaitForever); // w8 semaphore forever

//    /* Ensure exclusive access to the USBFS Tx. */
//    if (xUSBTxMutex != NULL && osOK == osMutexWait(xUSBTxMutex, 5)) {
//      while (USBD_OK != CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING)))
//        osDelay(1);
//
//      osMutexRelease(xUSBTxMutex);
//    }

    /* Ensure exclusive access to the USBFS Tx. */
    if (TelNetConn != NULL) {
      /* Send the AP.buffers.STRING to the telnet connection*/
      netconn_write(TelNetConn,
                    strToView,
                    strlen(strToView),
                    NETCONN_COPY);
    }

//    /* Store strToView to the SD card */
//    if (xBinSem_SDHandle != NULL) {
//      osSemaphoreRelease(xBinSem_SDHandle);
//    }

    /* Send strToView to the UDP syslog server */
    if (xBinSem_UDPSysLogHandle != NULL) {
      osSemaphoreRelease(xBinSem_UDPSysLogHandle);
    }
  }
}

/*----------------------------------------------------------------------------*/
void vStartViewStringTask() {
  LCD_UsrLog("vStartViewStringTask()\n");
  /* definition and creation of xBinSem_... */
  osSemaphoreDef(xBinSem_viewString);
  xBinSem_viewStringHandle = osSemaphoreCreate(osSemaphore(xBinSem_viewString), 1);
  /* take semaphore ... */
  osSemaphoreWait(xBinSem_viewStringHandle, 0);
  /* Create that task */
  osThreadDef(STR_VIEW,
              task,
              osPriorityNormal,
              0,
              2 * configMINIMAL_STACK_SIZE);
  osThreadCreate(osThread(STR_VIEW), NULL);
  /*****************************************************************************
  Register an example set of CLI commands.
  Then start the tasks (TCP and USB) that manages the CLI.
  *****************************************************************************/
  vRegisterSampleCLICommands();
}

void stringCatAndView(char *str) {
  sprintf(strToView, "%s 20%02u/%02u/%02u %02u:%02u:%02u | %s\r",
          weekDays[date.WeekDay],
          date.Year,
          date.Month,
          date.Date,
          time.Hours,
          time.Minutes,
          time.Seconds,
          str);
  osSemaphoreRelease(xBinSem_viewStringHandle);
}