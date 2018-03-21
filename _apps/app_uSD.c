/**
  ******************************************************************************
    @file    app_uSD.c
    @author  PavelB
    @version V1.0
    @date    20-March-2017
    @brief   uSD card
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "fatfs.h"
#include "main.h"
#include "app_uSD.h"
#include "app_usbFS.h"
#include "lcd_log.h"

/* Private define ------------------------------------------------------------*/
#define FILEMGR_LIST_DEPDTH                        100
#define FILEMGR_FILE_NAME_SIZE                     40
#define FILEMGR_FULL_PATH_SIZE                     256
#define FILEMGR_MAX_LEVEL                          4
#define FILETYPE_DIR                               0
#define FILETYPE_FILE                              1

/* Private typedef -----------------------------------------------------------*/
typedef struct _FILELIST_LineTypeDef {
    uint8_t type;
    uint8_t name[FILEMGR_FILE_NAME_SIZE];
} FILELIST_LineTypeDef;

typedef struct _FILELIST_FileTypeDef {
    FILELIST_LineTypeDef  file[FILEMGR_LIST_DEPDTH] ;
    uint16_t              ptr;
} FILELIST_FileTypeDef;

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern char SD_Path[4];  /* SD logical drive path */
extern apSettings_t AP;
extern RTC_TimeTypeDef AP_Time;
extern RTC_DateTypeDef AP_Date;
extern RTC_HandleTypeDef hrtc;
extern osSemaphoreId xBinSem_uSDHandle;
extern osSemaphoreId xBinSem_USBHandle;

FATFS SD_FatFs;
FILELIST_FileTypeDef FileList;

uint16_t NumObs = 0;
uint8_t sect[4096];
uint32_t bytesRD = 0, bytesWR = 0; /* File write/read counts */

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
uint32_t openBMP(uint8_t *ptr, const char *fName);
FRESULT storageParse(void);
uint8_t showFiles(void);

/**
    @brief  uSD Process
    @param  argument: network interface
    @retval None
*/
void uSD_thread(void const *argument) {
    BSP_SD_CardInfo uSD;
    char fNameConfig[] = "_20170417_12-00-00_config.TXT";
    char fNameLog[]    = "20170417_12-00-00_log.TXT";
    FIL fConfig, fLog;                    /* File object */
//  FRESULT res;                          /* FatFs function common result code */

    for (;;) {
        if (SD_NOT_PRESENT == BSP_SD_IsDetected()) {
            LCD_UsrLog("uSD | no disk!\n");
            sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | No disk detected\r\n",
                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
            SEND_STRING_TO_USB();

            while (SD_NOT_PRESENT == BSP_SD_IsDetected()) {
                osDelay(100);
            }
        } else {
            // update values of structures
            HAL_RTC_GetTime(&hrtc, &AP_Time, RTC_FORMAT_BIN);
            HAL_RTC_GetDate(&hrtc, &AP_Date, RTC_FORMAT_BIN);

            /* init code for FATFS */
            switch (BSP_SD_Init()) {
                case MSD_OK:
                    BSP_SD_ITConfig();
                    BSP_SD_GetCardInfo(&uSD);
                    LCD_UsrLog("uSD | volume = %d Mb\n", (uSD.BlockNbr * uSD.BlockSize) >> 20);
                    sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | volume = %d Mb\r\n",
                            AP_Date.Year, AP_Date.Month, AP_Date.Date,
                            AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds,
                            (uSD.BlockNbr * uSD.BlockSize) >> 20);
                    SEND_STRING_TO_USB();
                    osDelay(1);

                    /* Link the uSD card I/O driver */
                    if (FR_OK != f_mount(&SD_FatFs, (TCHAR const *)SD_Path, 0)) {
                        LCD_UsrLog("uSD | Error while mounting FatFs\n");
                        sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | Error while mounting FatFs\r\n",
                                AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
                        SEND_STRING_TO_USB();
                        osDelay(1);
                    } else {
                        LCD_DbgLog("uSD | FatFs mounted successfully\n");
                        sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | FatFs mounted successfully\r\n",
                                AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
                        SEND_STRING_TO_USB();
                        osDelay(1);
                        AP.uSDConnected = 1;

                        while (AP.uSDConnected) {
                            osDelay(1000);
                            // create file names with current date&time
                            sprintf(fNameConfig, "config_20%02u%02u%02u_%02u-%02u-%02u.txt",
                                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
                            sprintf(fNameLog, "log_20%02u%02u%02u_%02u-%02u-%02u.txt",
                                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);

                            // open files with tihs file names
                            if (FR_OK != f_open(&fConfig, fNameConfig, FA_CREATE_ALWAYS | FA_WRITE) ||
                                    FR_OK != f_open(&fLog, fNameLog, FA_CREATE_ALWAYS | FA_WRITE)) {
                                LCD_UsrLog("uSD | Can't open files:\n");
                                LCD_UsrLog("'%s'\n", fNameConfig);
                                LCD_UsrLog("'%s'\n", fNameLog);
                                sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | Can't open files: '%s', '%s'\r\n",
                                        AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                        AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds,
                                        fNameConfig, fNameLog);
                                SEND_STRING_TO_USB();
                            } else {
                                LCD_UsrLog("uSD | Open files:\n");
                                LCD_UsrLog("'%s'\n", fNameConfig);
                                LCD_UsrLog("'%s'\n", fNameLog);
                                sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | Open files: '%s', '%s'\r\n",
                                        AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                        AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds,
                                        fNameConfig, fNameLog);
                                SEND_STRING_TO_USB();

                                while (AP.uSDConnected) {
                                    //osSemaphoreWait(xBinSem_uSDHandle, osWaitForever); // w8 semaphore forever
                                    osDelay(5000);
                                    // update values of structures
                                    HAL_RTC_GetTime(&hrtc, &AP_Time, RTC_FORMAT_BIN);
                                    HAL_RTC_GetDate(&hrtc, &AP_Date, RTC_FORMAT_BIN);
                                    sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | ***TEST WRITE***\r\n",
                                            AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                            AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);

                                    // проверяем есть ли карта в слоте
                                    if (SD_PRESENT != BSP_SD_IsDetected()) {
                                        // Close text files
                                        f_close(&fConfig);
                                        f_close(&fLog);
                                        AP.uSDConnected = 0;
                                        LCD_UsrLog("uSD | Files saved, disk ejected\n");
                                        sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | Files saved, disk ejected\r\n",
                                                AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                                AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
                                        SEND_STRING_TO_USB();
                                    } else {
                                        // write .STRING to file
                                        if (f_write(&fConfig, AP.buffers.STRING, strlen(AP.buffers.STRING), (void *)&bytesWR) ||
                                                f_write(&fLog,    AP.buffers.STRING, strlen(AP.buffers.STRING), (void *)&bytesWR)) {
                                            LCD_UsrLog("uSD | write error\n");
                                            sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | write error\r\n",
                                                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
                                        } else {
                                            sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | write successfully\r\n",
                                                    AP_Date.Year, AP_Date.Month, AP_Date.Date,
                                                    AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
                                        }

                                        SEND_STRING_TO_USB();
                                    }
                                }
                            }
                        }
                    }

                    break;

                case MSD_ERROR:
                    LCD_UsrLog("uSD | unknown error\n");
                    sprintf(AP.buffers.STRING, "20%02u/%02u/%02u %02u:%02u:%02u | uSD | unknown error\r\n",
                            AP_Date.Year, AP_Date.Month, AP_Date.Date,
                            AP_Time.Hours, AP_Time.Minutes, AP_Time.Seconds);
                    SEND_STRING_TO_USB();
                    break;
            }

            // pause 1 sec before next try
            osDelay(1000);
        }
    }
}

/**
    @brief  Shows audio file (*.wav) on the root
    @param  None
    @retval None
*/
uint8_t showFiles(void) {
    uint8_t i = 0;
    uint8_t line_idx = 0;

    if (FR_OK == storageParse()) {
        if (FileList.ptr > 0) {
            LCD_UsrLog("uSD | file(s) [ROOT]:\n");

            for (i = 0; i < FileList.ptr; i++) {
                line_idx++;
                //LCD_DbgLog("   |__");
                LCD_DbgLog((char *)FileList.file[i].name);
                LCD_DbgLog("\n");
            }

            LCD_UsrLog("End of files list\n");
            return 0;
        }

        return 1;
    }

    return 2;
}

/**
    @brief  Copies disk content in the explorer list.
    @param  None
    @retval Operation result
*/
FRESULT storageParse(void) {
    FRESULT res = FR_OK;
    FILINFO fno;
    DIR dir;
    char *fn;
#if _USE_LFN
    static char lfn[_MAX_LFN];
    fno.lfname = lfn;
    fno.lfsize = sizeof(lfn);
#endif
    res = f_opendir(&dir, SD_Path);
    FileList.ptr = 0;

    if (res == FR_OK) {
        while (1) {
            res = f_readdir(&dir, &fno);

            if (res != FR_OK || fno.fname[0] == 0)
            { break; }

            if (fno.fname[0] == '.')
            { continue; }

#if _USE_LFN
            fn = *fno.lfname ? fno.lfname : fno.fname;
#else
            fn = fno.fname;
#endif

            if (FileList.ptr < FILEMGR_LIST_DEPDTH) {
                if ((fno.fattrib & AM_DIR) == 0) {
                    strncpy((char *)FileList.file[FileList.ptr].name, (char *)fn, FILEMGR_FILE_NAME_SIZE);
                    FileList.file[FileList.ptr].type = FILETYPE_FILE;
                    FileList.ptr++;
                }
            }
        }
    }

    NumObs = FileList.ptr;
    f_closedir(&dir);
    return res;
}

uint32_t openBMP(uint8_t *ptr, TCHAR const *fName) {
    uint32_t sz = 0, i = 0, j = 0;
    static uint32_t bmpAddr;
    FIL file;

    if (FR_OK != f_open(&file, fName, FA_READ)) {
//    LCD_UsrLog("Can't open file %s\n", fName);
        return 0;
    } else {
        if (FR_OK != f_read(&file, sect, 30, (UINT *)&bytesRD)) {
//      LCD_UsrLog("Can't read file %s\n", fName);
            f_close(&file);
            return 0;
        } else {
            bmpAddr = (uint32_t)sect;
            // get bitmap size
            sz  =  *(uint16_t *)(bmpAddr + 2);
            sz |= (*(uint16_t *)(bmpAddr + 4)) << 16;
            // get bitmap data adress offset
//      i  =  *(uint16_t *)(bmpAddr+10);
//      i |= (*(uint16_t *)(bmpAddr+12))<<16;
            f_close(&file);
            f_open(&file, fName, FA_READ);

            do {
                i = 4096;

                if (sz < 4096) {
                    i = sz;
                }

                f_lseek(&file, j);
                f_read(&file, sect, i, (UINT *)&bytesRD);

                if (i == bytesRD) {
                    memcpy((void *)(ptr + j), (void *)sect, i);
                    j += i;
                }

                sz -= i;
            } while (sz > 0);

            f_close(&file);
        }
    }

    return 1;
}
