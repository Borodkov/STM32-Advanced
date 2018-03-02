/******************************************************************************
 *
 *
 ******************************************************************************/

/* OS includes. */
#include "cmsis_os.h"

/* Standard includes. */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

// 19/09/2017 - add Tftp feature
//#include "tftpserver.h"

// 19/09/2017 - add IAP feature
//#include "IAP_tftpserver.h"
//#include "IAP_httpserver.h"
/*-----------------------------------------------------------*/

#ifndef configINCLUDE_QUERY_HEAP_COMMAND
#define configINCLUDE_QUERY_HEAP_COMMAND 0
#endif
/*-----------------------------------------------------------*/

//extern apSettings_t AP;
// extern RTC_TimeTypeDef AP_Time;
// extern RTC_DateTypeDef AP_Date;
// extern RTC_HandleTypeDef hrtc;
extern osSemaphoreId xBinSem_BTNHandle;
/*-----------------------------------------------------------*/

extern void vStartHTTPServerTask(void);
extern void vStartUDPSysLogTask(const char *sServerIP);
/*-----------------------------------------------------------*/

/*
 * The function that registers the commands that are defined within this file.
 */
void vRegisterSampleCLICommands(void);

/*
 * Implements the run-time-stats command.
 */
#if ((configGENERATE_RUN_TIME_STATS == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS > 0))
static BaseType_t prvRunTimeStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
#endif

/*
 * Implements the task-stats command.
 */
#if ((configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS > 0))
static BaseType_t prvTaskStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
#endif

/*
 * Implements the "query heap" command.
 */
#if (configINCLUDE_QUERY_HEAP_COMMAND == 1)
static BaseType_t prvQueryHeapCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
#endif

static BaseType_t prvSetCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvBtnPressCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);
static BaseType_t prvStartTaskCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);


/* Structure that defines the "task-stats" command line command.  This generates
a table that gives information on each task in the system. */
#if ((configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS > 0))
static const CLI_Command_Definition_t xTaskStats = {
  "task-stats", /* The command string to type. */
  "\r\ntask-stats:\r\n Displays a table showing the state of each FreeRTOS task\r\n",
  prvTaskStatsCommand, /* The function to run. */
  0                    /* No parameters are expected. */
};
#endif

/* Structure that defines the "run-time-stats" command line command.
   This generates a table that shows how much run time each task has */
#if ((configGENERATE_RUN_TIME_STATS == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS > 0))
static const CLI_Command_Definition_t xRunTimeStats = {
  "run-time-stats", /* The command string to type. */
  "\r\nrun-time-stats:\r\n Displays a table showing how much processing time each FreeRTOS task has used\r\n",
  prvRunTimeStatsCommand, /* The function to run. */
  0                       /* No parameters are expected. */
};
#endif

#if (configINCLUDE_QUERY_HEAP_COMMAND == 1)
/* Structure that defines the "query_heap" command line command. */
static const CLI_Command_Definition_t xQueryHeap = {
  "query-heap",
  "\r\nquery-heap:\r\n Displays the free heap space, and minimum ever free heap space.\r\n",
  prvQueryHeapCommand, /* The function to run. */
  0                    /* No parameters are expected. */
};
#endif /* configQUERY_HEAP_COMMAND */

/* */
static const CLI_Command_Definition_t xBtnPress = {
  "btnPress",
  "\r\nbtnPress:\r\n virtual press button\r\n",
  prvBtnPressCommand, /* The function to run. */
  1                   /* One parameter is expected. */
};

/* */
static const CLI_Command_Definition_t xStartTask = {
  "startTask",
  "\r\nstartTask:\r\n Starts OS Task with <TASKNAME>:\r\n <HTTP>\r\n <TFTP>\r\n <SYSLOG> <server IP> (e.g. <192.168.001.101:514>)\r\n",
  prvStartTaskCommand, /* The function to run. */
  -1                   /* The user can enter any number of commands. */
};

/* */
static const CLI_Command_Definition_t xSet = {
  "set",
  "\r\nset <...>:\r\n Set following params:\r\n <date> <current date> (e.g. <set date 2017/01/01>)\r\n <time> <current time> (e.g. <set time 10:00:00>)\r\n",
  prvSetCommand, /* The function to run. */
  -1             /* The user can enter any number of commands. */
};

/*-----------------------------------------------------------*/

void vRegisterSampleCLICommands(void) {
  /* Register all the command line commands defined immediately above. */
  #if ((configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS > 0))
  {
    FreeRTOS_CLIRegisterCommand(&xTaskStats);
  }
  #endif
  #if ((configGENERATE_RUN_TIME_STATS == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS > 0))
  {
    FreeRTOS_CLIRegisterCommand(&xRunTimeStats);
  }
  #endif
  #if (configINCLUDE_QUERY_HEAP_COMMAND == 1)
  {
    FreeRTOS_CLIRegisterCommand(&xQueryHeap);
  }
  #endif
  FreeRTOS_CLIRegisterCommand(&xSet);
  FreeRTOS_CLIRegisterCommand(&xBtnPress);
  FreeRTOS_CLIRegisterCommand(&xStartTask);
}
/*-----------------------------------------------------------*/

#if ((configUSE_TRACE_FACILITY == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS > 0))

static BaseType_t prvTaskStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  const char *const pcHeader = " State\tPrior\tStack\t#\r\n************************************************\r\n";
  BaseType_t xSpacePadding;
  /* Remove compile time warnings about unused parameters, and check the
     write buffer is not NULL.  NOTE - for simplicity, this example assumes the
     write buffer length is adequate, so does not check for buffer overflows. */
  (void)pcCommandString;
  (void)xWriteBufferLen;
  configASSERT(pcWriteBuffer);
  /* Generate a table of task stats. */
  strcpy(pcWriteBuffer, "Task");
  pcWriteBuffer += strlen(pcWriteBuffer);
  /* Minus three for the null terminator and half the number of characters in
    "Task" so the column lines up with the centre of the heading. */
  configASSERT(configMAX_TASK_NAME_LEN > 3);

  for (xSpacePadding = strlen("Task"); xSpacePadding < (configMAX_TASK_NAME_LEN - 3); xSpacePadding++) {
    /* Add a space to align columns after the task's name. */
    *pcWriteBuffer = ' ';
    pcWriteBuffer++;
  }

  strcpy(pcWriteBuffer, pcHeader);
  vTaskList(pcWriteBuffer + strlen(pcHeader));
  /* There is no more data to return after this single string, so return pdFALSE. */
  return pdFALSE;
}

#endif /* ( ( configUSE_TRACE_FACILITY == 1 ) && ( configUSE_STATS_FORMATTING_FUNCTIONS > 0 ) ) */
/*-----------------------------------------------------------*/

#if ((configGENERATE_RUN_TIME_STATS == 1) && (configUSE_STATS_FORMATTING_FUNCTIONS > 0))

static BaseType_t prvRunTimeStatsCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  const char *const pcHeader = "Task\t\tAbs Time[us]\t% Time\r\n****************************************\r\n";
  /* Remove compile time warnings about unused parameters, and check the
     write buffer is not NULL.  NOTE - for simplicity, this example assumes the
     write buffer length is adequate, so does not check for buffer overflows. */
  (void)pcCommandString;
  (void)xWriteBufferLen;
  configASSERT(pcWriteBuffer);
  /* Generate a table of task stats. */
  strcpy(pcWriteBuffer, pcHeader);
  vTaskGetRunTimeStats(pcWriteBuffer + strlen(pcHeader));
  /* There is no more data to return after this single string, so return pdFALSE. */
  return pdFALSE;
}

#endif /* configGENERATE_RUN_TIME_STATS */
/*-----------------------------------------------------------*/

#if (configINCLUDE_QUERY_HEAP_COMMAND == 1)

static BaseType_t prvQueryHeapCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  /* Remove compile time warnings about unused parameters, and check the
     write buffer is not NULL.  NOTE - for simplicity, this example assumes the
     write buffer length is adequate, so does not check for buffer overflows. */
  (void)pcCommandString;
  (void)xWriteBufferLen;
  configASSERT(pcWriteBuffer);
  sprintf(pcWriteBuffer, "Current free heap %d bytes, minimum ever free heap %d bytes\r\n", (int)xPortGetFreeHeapSize(), (int)xPortGetMinimumEverFreeHeapSize());
  /* There is no more data to return after this single string, so return pdFALSE. */
  return pdFALSE;
}

#endif /* configINCLUDE_QUERY_HEAP */
/*-----------------------------------------------------------*/

static BaseType_t prvSetCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
//  const char *pcParameter;
//  BaseType_t xParameterStringLength;
//
//  /* Remove compile time warnings about unused parameters, and check the
//     write buffer is not NULL.  NOTE - for simplicity, this example assumes the
//     write buffer length is adequate, so does not check for buffer overflows. */
//  (void)pcCommandString;
//  (void)xWriteBufferLen;
//  configASSERT(pcWriteBuffer);
//
//  /* Obtain the parameter string. */
//  pcParameter = FreeRTOS_CLIGetParameter(
//      pcCommandString,        /* The command string itself. */
//      1,                      /* Return the first parameter. */
//      &xParameterStringLength /* Store the parameter string length. */
//      );
//
//  if (pcParameter != NULL)
//  {
//    if (strncmp(pcParameter, "date", strlen("date")) == 0)
//    {
//      /* Obtain the parameter string. */
//      pcParameter = FreeRTOS_CLIGetParameter(
//          pcCommandString,        /* The command string itself. */
//          2,                      /* Return the second parameter. */
//          &xParameterStringLength /* Store the parameter string length. */
//          );
//
//      if (pcParameter != NULL)
//      {
//        unsigned int YY,MM,DD;
//        sscanf(pcParameter, "20%u/%u/%u", &YY, &MM, &DD);
//
//        AP_Date.Year    = (uint8_t)YY;
//        AP_Date.Month   = (uint8_t)MM;
//        AP_Date.Date    = (uint8_t)DD;
//        HAL_RTC_SetDate(&hrtc, &AP_Date, RTC_FORMAT_BIN);
//
//        sprintf(pcWriteBuffer, "Date have been changed to '20%02u/%02u/%02u'\r\n", YY, MM, DD);
//      }
//      else
//      {
//        sprintf(pcWriteBuffer, "\r\nEnter <date> (e.g. <2017/01/01>)\r\n");
//      }
//    }
//    else if (strncmp(pcParameter, "time", strlen("time")) == 0)
//    {
//      /* Obtain the parameter string. */
//      pcParameter = FreeRTOS_CLIGetParameter(
//          pcCommandString,        /* The command string itself. */
//          2,                      /* Return the second parameter. */
//          &xParameterStringLength /* Store the parameter string length. */
//          );
//
//      if (pcParameter != NULL)
//      {
//        unsigned int hh,mm,ss;
//        sscanf(pcParameter, "%u:%u:%u", &hh, &mm, &ss);
//
//        AP_Time.Hours   = (uint8_t)hh;
//        AP_Time.Minutes = (uint8_t)mm;
//        AP_Time.Seconds = (uint8_t)ss;
//        HAL_RTC_SetTime(&hrtc, &AP_Time, RTC_FORMAT_BIN);
//
//        sprintf(pcWriteBuffer, "Time have been changed to '%02u:%02u:%02u'\r\n", hh, mm, ss);
//      }
//      else
//      {
//        sprintf(pcWriteBuffer, "\r\nEnter <time> (e.g. <10:00:00>)\r\n");
//      }
//    }
//    else
//    {
//      sprintf(pcWriteBuffer, "Valid parameters are:\r\n <date> <current date> (e.g. <set date 2017/01/01>)\r\n <time> <current time> (e.g. <set time 10:00:00>)\r\n");
//    }
//  }
//  else
//  {
//    sprintf(pcWriteBuffer, "Valid parameters are:\r\n <date> <current date> (e.g. <set date 2017/01/01>)\r\n <time> <current time> (e.g. <set time 10:00:00>)\r\n");
//  }
  /* There is no more data to return after this single string, so return pdFALSE. */
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvBtnPressCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
//  const char *pcParameter;
//  BaseType_t xParameterStringLength;
//
//  /* Remove compile time warnings about unused parameters, and check the
//     write buffer is not NULL.  NOTE - for simplicity, this example assumes the
//     write buffer length is adequate, so does not check for buffer overflows. */
//  (void)pcCommandString;
//  (void)xWriteBufferLen;
//  configASSERT(pcWriteBuffer);
//
//  /* Obtain the parameter string. */
//  pcParameter = FreeRTOS_CLIGetParameter(
//      pcCommandString,        /* The command string itself. */
//      1,                      /* Return the first parameter. */
//      &xParameterStringLength /* Store the parameter string length. */
//      );
//
//  if (pcParameter != NULL)
//  {
//    if (strncmp(pcParameter, "ok", strlen("ok")) == 0)
//    {
////      AP.pressedBTN |= BTN_OK_Pin;
////      osSemaphoreRelease(xBinSem_BTNHandle);
//    }
//    else if (strncmp(pcParameter, "up", strlen("up")) == 0)
//    {
////      AP.pressedBTN |= BTN_UP_Pin;
////      osSemaphoreRelease(xBinSem_BTNHandle);
//    }
//    else if (strncmp(pcParameter, "down", strlen("down")) == 0)
//    {
////      AP.pressedBTN |= BTN_DOWN_Pin;
////      osSemaphoreRelease(xBinSem_BTNHandle);
//    }
//    else
//    {
//      sprintf(pcWriteBuffer, "Valid parameters are:\r\n <ok> or <up> or <down>\r\n");
//    }
//  }
//  else
//  {
//    sprintf(pcWriteBuffer, "Valid parameters are:\r\n <ok> or <up> or <down>\r\n");
//  }
  /* There is no more data to return after this single string, so return pdFALSE. */
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvStartTaskCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
//  const char *pcParameter;
//  BaseType_t xParameterStringLength;
//
//  /* Remove compile time warnings about unused parameters, and check the
//  write buffer is not NULL.  NOTE - for simplicity, this example assumes the
//  write buffer length is adequate, so does not check for buffer overflows. */
//  (void)pcCommandString;
//  (void)xWriteBufferLen;
//  configASSERT(pcWriteBuffer);
//
//  /* Obtain the parameter string. */
//  pcParameter = FreeRTOS_CLIGetParameter(
//      pcCommandString,        /* The command string itself. */
//      1,                      /* Return the first parameter. */
//      &xParameterStringLength /* Store the parameter string length. */
//      );
//
//  if (pcParameter != NULL)
//  {
//    /* There are only two valid parameter values. */
//    if (strncmp(pcParameter, "HTTP", strlen("HTTP")) == 0)
//    {
//      /* HTTP Server */
//      vStartHTTPServerTask();
//    }
//    else if (strncmp(pcParameter, "TFTP", strlen("TFTP")) == 0)
//    {
//      /* Initialize the TFTP server */
////      tftpd_init();
////
////        #ifdef USE_IAP_HTTP
////          /* Initialize the webserver module */
////          IAP_httpd_init();
////        #endif
////
////        #ifdef USE_IAP_TFTP
////          /* Initialize the TFTP server */
////          IAP_tftpd_init();
////        #endif
//    }
//    else if (strncmp(pcParameter, "SYSLOG", strlen("SYSLOG")) == 0)
//    {
//      /* Obtain the parameter string. */
//      pcParameter = FreeRTOS_CLIGetParameter(
//          pcCommandString,        /* The command string itself. */
//          2,                      /* Return the second parameter. */
//          &xParameterStringLength /* Store the parameter string length. */
//          );
//
//      if (pcParameter != NULL)
//      {
//        vStartUDPSysLogTask(pcParameter);
//
//        *pcWriteBuffer = '\0';
//      }
//      else
//      {
//        sprintf(pcWriteBuffer, "\r\nEnter <server IP> (e.g. <192.168.001.101:514>)\r\n");
//      }
//    }
//    else
//    {
//      sprintf(pcWriteBuffer, "Valid parameters are:\r\n<HTTP>\r\n<SYSLOG> + server IP <192.168.001.101:514>\r\n");
//    }
//  }
//  else
//  {
//    sprintf(pcWriteBuffer, "Valid parameters are:\r\n<HTTP>\r\n<SYSLOG> + server IP <192.168.001.101:514>\r\n");
//  }
  /* There is no more data to return after this single string, so return pdFALSE. */
  return pdFALSE;
}
/*-----------------------------------------------------------*/
