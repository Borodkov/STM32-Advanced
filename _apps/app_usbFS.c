/**
  ******************************************************************************
    @file    app_usbFS.c
    @author  PavelB
    @version V1.0
    @date    20-March-2017
    @brief   virtual com port on USB FS
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "app_usbFS.h"
#include "lcd_log.h"

/* Private define ------------------------------------------------------------*/
/* Private typedef -----------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern USBD_HandleTypeDef hUsbDeviceFS;
extern apSettings_t AP;
extern RTC_TimeTypeDef AP_Time;
extern RTC_DateTypeDef AP_Date;
extern RTC_HandleTypeDef hrtc;
extern osSemaphoreId xBinSem_uSDHandle;
extern osSemaphoreId xBinSem_USBHandle;
extern osSemaphoreId xBinSem_ETHHandle;
extern osSemaphoreId xBinSem_BTNHandle;
extern uint16_t btnFlags;

/* Private function prototypes -----------------------------------------------*/
uint32_t checkUSBCommand();

/**
    @brief  uSD Process
    @param  argument: network interface
    @retval None
*/
void usbFS_thread(void const *argument) {
    LCD_UsrLog("USB | wait connection...\n");

    while (AP.usbConnected == 0) {
        osDelay(100);
    }

    //audio_storeSounds(SND_USBCON);
    LCD_UsrLog("USB | connection have been esteblished!\n");
    HAL_RTC_GetTime(&hrtc, &AP_Time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &AP_Date, RTC_FORMAT_BIN);
    sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | USB Connected!\r\n",
            AP_Date.Year,
            AP_Date.Month,
            AP_Date.Date,
            AP_Time.Hours,
            AP_Time.Minutes,
            AP_Time.Seconds);
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    sprintf(AP.buffers.STRING, "************************************\r\n");
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    osDelay(1);
    sprintf(AP.buffers.STRING, "List of commands:\r\n");
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    osDelay(1);
    sprintf(AP.buffers.STRING, "-next step\r\n");
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    osDelay(1);
    sprintf(AP.buffers.STRING, "-set date yyyy/mm/dd hh:mm:ss\r\n");
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    osDelay(1);
    sprintf(AP.buffers.STRING, "-set time hh:mm:ss\r\n");
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    osDelay(1);
    sprintf(AP.buffers.STRING, "-set serv 255.255.255.255\r\n");
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    osDelay(1);
    sprintf(AP.buffers.STRING, "-set bOK\r\n-set bUP\r\n-set bDOWN\r\n");
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    osDelay(1);
    sprintf(AP.buffers.STRING, "-settings\r\n");
    CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
    osDelay(1);
    sprintf(AP.buffers.STRING, "************************************\r\n");

    /* Infinite loop */
    for (;;) {
        // commands start with '-'
        if (AP.buffers.STRING[0] == '-') {
            //audio_storeSounds(SND_GET_CMD);
            if (checkUSBCommand()) {
                sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | Unknown command. Try again with commands from list...\r\n",
                        AP_Date.Year, AP_Date.Month, AP_Date.Date,
                        AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
            } else {
                for (uint32_t i = 0; i < sizeof(AP.buffers.STRING); i++) {
                    AP.buffers.STRING[i] = 0;
                }
            }
        }

        if (strlen(AP.buffers.STRING) > 0) {
            CDC_Transmit_FS((uint8_t *)AP.buffers.STRING, strlen(AP.buffers.STRING));
        }

        osSemaphoreWait(xBinSem_USBHandle, osWaitForever); // w8 semaphore forever
    }
}

/* Private function ----------------------------------------------------------*/
uint32_t checkUSBCommand() {
    // update values of structures
    HAL_RTC_GetTime(&hrtc, &AP_Time, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &AP_Date, RTC_FORMAT_BIN);

    // Функция, обратная sprintf — это sscanf. Она принимает строку с данными и текстом,
    // пытается её разобрать в соответствии со строкой форматирования, и записывает
    // найденные данные по переданным адресам. Простейший пример:
    //
    // float lat, lon;
    // sscanf("GPS lat: 63.321, lon: 37.867", "GPS lat: %f, lon: %f", &lat, &lon);
    //
    // Здесь действуют все те же самые правила составления идентификаторов,
    // что и в printf, и есть все те же самые функции вроде scanf, fscanf, sscanf.

    // EXAMPLE of terminal commands:
    //           1111111111222222222
    // 01234567890123456789012345678
    // -set date 2017/01/01 12:00:00

    //           111111111122222222223333333333444444444455555555556666666666
    // 0123456789012345678901234567890123456789012345678901234567890123456789
    // -settings: RE | 1 0 0 0 | T = 0015 mins |
    // -settings: AB | 0 0 0 0 | T = 0030 mins |
    // -settings: ED | 1 0 1 1 | T = 0060 mins | Thr1 = 22 | Thr2 = 100

    if (strstr(AP.buffers.STRING, "next step")) {
        osSemaphoreRelease(xBinSem_BTNHandle);
    } else if (strstr(AP.buffers.STRING, "bOK")) {
        //btnFlags |= BTN_OK_Pin;
        //audio_storeSounds(SND_BTN_OK);
        osSemaphoreRelease(xBinSem_BTNHandle);
    } else if (strstr(AP.buffers.STRING, "bUP")) {
        //btnFlags |= BTN_UP_Pin;
        //audio_storeSounds(SND_BTN_UP);
        osSemaphoreRelease(xBinSem_BTNHandle);
    } else if (strstr(AP.buffers.STRING, "bDOWN")) {
        //btnFlags |= BTN_DOWN_Pin;
        //audio_storeSounds(SND_BTN_DOWN);
        osSemaphoreRelease(xBinSem_BTNHandle);
    } else if (strstr(AP.buffers.STRING, "date")) {
        unsigned int YY, MM, DD, hh, mm, ss;
        sscanf(AP.buffers.STRING, "-set date 20%u/%u/%u %u:%u:%u", &YY, &MM, &DD, &hh, &mm, &ss);
        AP_Date.Year    = (uint8_t)YY;
        AP_Date.Month   = (uint8_t)MM;
        AP_Date.Date    = (uint8_t)DD;
        AP_Time.Hours   = (uint8_t)hh;
        AP_Time.Minutes = (uint8_t)mm;
        AP_Time.Seconds = (uint8_t)ss;
        // set new date and time
        HAL_RTC_SetDate(&hrtc, &AP_Date, RTC_FORMAT_BIN);
        HAL_RTC_SetTime(&hrtc, &AP_Time, RTC_FORMAT_BIN);
    } else if (strstr(AP.buffers.STRING, "time")) {
        unsigned int hh, mm, ss;
        sscanf(AP.buffers.STRING, "-set time %u:%u:%u", &hh, &mm, &ss);
        AP_Time.Hours   = (uint8_t)hh;
        AP_Time.Minutes = (uint8_t)mm;
        AP_Time.Seconds = (uint8_t)ss;
        // set new time
        HAL_RTC_SetTime(&hrtc, &AP_Time, RTC_FORMAT_BIN);
    } else if (strstr(AP.buffers.STRING, "serv")) {
        unsigned int addr0, addr1, addr2, addr3;
        sscanf(AP.buffers.STRING, "-set serv %u.%u.%u.%u", &addr0, &addr1, &addr2, &addr3);
        AP.ethServIP[0] = (uint8_t)addr0;
        AP.ethServIP[1] = (uint8_t)addr1;
        AP.ethServIP[2] = (uint8_t)addr2;
        AP.ethServIP[3] = (uint8_t)addr3;
        osSemaphoreRelease(xBinSem_ETHHandle);
    } else if (strstr(AP.buffers.STRING, "settings")) {
//    // sensors
//    AP.buffers.UART2[1] = 0;
//    if (AP.buffers.STRING[16] == '1') AP.buffers.UART2[1] |= BSP_BIT_GERKON1;
//    if (AP.buffers.STRING[18] == '1') AP.buffers.UART2[1] |= BSP_BIT_GERKON2;
//    if (AP.buffers.STRING[20] == '1') AP.buffers.UART2[1] |= BSP_BIT_PIR;
//    if (AP.buffers.STRING[22] == '1') AP.buffers.UART2[1] |= BSP_BIT_ACL;
//
//    // T (mins)
//    uint16_t tmp = 0;
//    tmp  = str2Num2(AP.buffers.STRING[30],AP.buffers.STRING[31]) * 100;
//    tmp += str2Num2(AP.buffers.STRING[32],AP.buffers.STRING[33]);
//    AP.buffers.UART2[2] = (uint8_t)(tmp);
//    AP.buffers.UART2[3] = (uint8_t)(tmp >> 8);
//
//    // Thr1 & Thr2
//    AP.buffers.UART2[4] = str2Num2(AP.buffers.STRING[49],AP.buffers.STRING[50]);
//    AP.buffers.UART2[5] = str2Num3(AP.buffers.STRING[61],AP.buffers.STRING[62],AP.buffers.STRING[63]);
        if (AP.buffers.STRING[11] == 'R' && AP.buffers.STRING[12] == 'E') {
            sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | Set RE settings:\r\n",
                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
        } else if (AP.buffers.STRING[11] == 'A' && AP.buffers.STRING[12] == 'B') {
            sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | Set AB settings:\r\n",
                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
        } else if (AP.buffers.STRING[11] == 'E' && AP.buffers.STRING[12] == 'D') {
            sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | Set ED settings:\r\n",
                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
        } else {
            sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | Unknown device\r\n",
                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
        }
    } else {
        return 1;
    }

    return 0;
}
