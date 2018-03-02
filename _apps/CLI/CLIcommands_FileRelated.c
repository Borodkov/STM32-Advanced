/******************************************************************************
 *
 *
 ******************************************************************************/

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"

/* Standard includes. */
#include <string.h>
#include <stdio.h>

/* FreeRTOS+CLI includes. */
#include "FreeRTOS_CLI.h"

/* fatFs includes. */
#include "ff.h"

extern FILELIST_FileTypeDef FileList;

#define cliNEW_LINE "\r\n"

/*******************************************************************************
 * See the URL in the comments within main.c for the location of the online
 * documentation.
 ******************************************************************************/

/*
 * Print out information on a single file.
 */
//static void prvCreateFileInfoString(char *pcBuffer, FF_FindData_t *pxFindStruct);

/*
 * Copies an existing file into a newly created file.
 */
static BaseType_t prvPerformCopy(const char *pcSourceFile,
                                 int32_t lSourceFileLength,
                                 const char *pcDestinationFile,
                                 char *pxWriteBuffer,
                                 size_t xWriteBufferLen);

/*
 * Implements the DIR command.
 */
static BaseType_t prvDIRCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*
 * Implements the CD command.
 */
static BaseType_t prvCDCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*
 * Implements the DEL command.
 */
static BaseType_t prvDELCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*
 * Implements the DEL command.
 */
static BaseType_t prvRMDIRCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*
 * Implements the TYPE command.
 */
static BaseType_t prvTYPECommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*
 * Implements the COPY command.
 */
static BaseType_t prvCOPYCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/*
 * Implements the PWD (print working directory) command.
 */
static BaseType_t prvPWDCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString);

/* Structure that defines the DIR command line command, which lists all the
files in the current directory. */
static const CLI_Command_Definition_t xDIR = {
  "dir", /* The command string to type. */
  "\r\ndir:\r\n Lists the files in the current directory\r\n",
  prvDIRCommand, /* The function to run. */
  0              /* No parameters are expected. */
};

/* Structure that defines the CD command line command, which changes the
working directory. */
static const CLI_Command_Definition_t xCD = {
  "cd", /* The command string to type. */
  "\r\ncd <dir name>:\r\n Changes the working directory\r\n",
  prvCDCommand, /* The function to run. */
  1             /* One parameter is expected. */
};

/* Structure that defines the TYPE command line command, which prints the
contents of a file to the console. */
static const CLI_Command_Definition_t xTYPE = {
  "type", /* The command string to type. */
  "\r\ntype <filename>:\r\n Prints file contents to the terminal\r\n",
  prvTYPECommand, /* The function to run. */
  1               /* One parameter is expected. */
};

/* Structure that defines the DEL command line command, which deletes a file. */
static const CLI_Command_Definition_t xDEL = {
  "del", /* The command string to type. */
  "\r\ndel <filename>:\r\n deletes a file (use rmdir to delete a directory)\r\n",
  prvDELCommand, /* The function to run. */
  1              /* One parameter is expected. */
};

/* Structure that defines the RMDIR command line command, which deletes a directory. */
static const CLI_Command_Definition_t xRMDIR = {
  "rmdir", /* The command string to type. */
  "\r\nrmdir <directory name>:\r\n deletes a directory\r\n",
  prvRMDIRCommand, /* The function to run. */
  1                /* One parameter is expected. */
};

/* Structure that defines the COPY command line command, which deletes a file. */
static const CLI_Command_Definition_t xCOPY = {
  "copy", /* The command string to type. */
  "\r\ncopy <source file> <dest file>:\r\n Copies <source file> to <dest file>\r\n",
  prvCOPYCommand, /* The function to run. */
  2               /* Two parameters are expected. */
};

/* Structure that defines the pwd command line command, which prints the current working directory. */
static const CLI_Command_Definition_t xPWD = {
  "pwd", /* The command string to type. */
  "\r\npwd:\r\n Print Working Directory\r\n",
  prvPWDCommand, /* The function to run. */
  0              /* No parameters are expected. */
};

/*-----------------------------------------------------------*/

void vRegisterFileSystemCLICommands(void) {
  /* Register all the command line commands defined immediately above. */
  FreeRTOS_CLIRegisterCommand(&xDIR);
  FreeRTOS_CLIRegisterCommand(&xCD);
  FreeRTOS_CLIRegisterCommand(&xTYPE);
  FreeRTOS_CLIRegisterCommand(&xDEL);
  FreeRTOS_CLIRegisterCommand(&xRMDIR);
  FreeRTOS_CLIRegisterCommand(&xCOPY);
  FreeRTOS_CLIRegisterCommand(&xPWD);
}
/*-----------------------------------------------------------*/

static BaseType_t prvTYPECommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  BaseType_t xParameterStringLength, xReturn = pdTRUE;
//  const char *pcParameter;
//  static FF_FILE *pxFile = NULL;
//  int iChar;
//  size_t xByte;
//  size_t xColumns = 50U;
//
//  /* Ensure there is always a null terminator after each character written. */
//  memset(pcWriteBuffer, 0x00, xWriteBufferLen);
//
//  /* Ensure the buffer leaves space for the \r\n. */
//  configASSERT(xWriteBufferLen > (strlen(cliNEW_LINE) * 2));
//  xWriteBufferLen -= strlen(cliNEW_LINE);
//
//  if (xWriteBufferLen < xColumns)
//  {
//    /* Ensure the loop that uses xColumns as an end condition does not
//    write off the end of the buffer. */
//    xColumns = xWriteBufferLen;
//  }
//
//  if (pxFile == NULL)
//  {
//    /* The file has not been opened yet.  Find the file name. */
//    pcParameter = FreeRTOS_CLIGetParameter(
//        pcCommandString,        /* The command string itself. */
//        1,                      /* Return the first parameter. */
//        &xParameterStringLength /* Store the parameter string length. */
//        );
//
//    /* Sanity check something was returned. */
//    configASSERT(pcParameter);
//
//    /* Attempt to open the requested file. */
//    pxFile = ff_fopen(pcParameter, "r");
//  }
//
//  if (pxFile != NULL)
//  {
//    /* Read the next chunk of data from the file. */
//    for (xByte = 0; xByte < xColumns; xByte++)
//    {
//      iChar = ff_fgetc(pxFile);
//
//      if (iChar == -1)
//      {
//        /* No more characters to return. */
//        ff_fclose(pxFile);
//        pxFile = NULL;
//        break;
//      }
//      else
//      {
//        pcWriteBuffer[xByte] = (char)iChar;
//      }
//    }
//  }
//
//  if (pxFile == NULL)
//  {
//    /* Either the file was not opened, or all the data from the file has
//    been returned and the file is now closed. */
//    xReturn = pdFALSE;
//  }
  strcat(pcWriteBuffer, cliNEW_LINE);
  return xReturn;
}
/*-----------------------------------------------------------*/

static BaseType_t prvCDCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
//  const char *pcParameter;
//  BaseType_t xParameterStringLength;
//  int iReturned;
//  size_t xStringLength;
//
//  /* Obtain the parameter string. */
//  pcParameter = FreeRTOS_CLIGetParameter(
//      pcCommandString,        /* The command string itself. */
//      1,                      /* Return the first parameter. */
//      &xParameterStringLength /* Store the parameter string length. */
//      );
//
//  /* Sanity check something was returned. */
//  configASSERT(pcParameter);
//
//  /* Attempt to move to the requested directory. */
//  iReturned = ff_chdir(pcParameter);
//
//  if (iReturned == FF_ERR_NONE)
//  {
//    sprintf(pcWriteBuffer, "In: ");
//    xStringLength = strlen(pcWriteBuffer);
//    ff_getcwd(&(pcWriteBuffer[xStringLength]), (unsigned char)(xWriteBufferLen - xStringLength));
//  }
//  else
//  {
//    sprintf(pcWriteBuffer, "Error");
//  }
  strcat(pcWriteBuffer, cliNEW_LINE);
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvDIRCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  BaseType_t xReturn = pdFALSE;
  static uint16_t currPtrInFileList = 0;

  if (FileList.ptr > 0) {
    /* Create a string that includes the file name, the file size and the
          attributes string. */
    if (currPtrInFileList < FileList.ptr) {
      sprintf(pcWriteBuffer, "%s ", FileList.file[currPtrInFileList++].name);
      xReturn = pdPASS;
    } else {
      // next time 'dir' start from the [0]
      currPtrInFileList = 0;
    }
  }

  strcat(pcWriteBuffer, cliNEW_LINE);
  return xReturn;
}
/*-----------------------------------------------------------*/

static BaseType_t prvRMDIRCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
//  const char *pcParameter;
//  BaseType_t xParameterStringLength;
//  int iReturned;
//
//  /* This function assumes xWriteBufferLen is large enough! */
//  (void)xWriteBufferLen;
//
//  /* Obtain the parameter string. */
//  pcParameter = FreeRTOS_CLIGetParameter(
//      pcCommandString,        /* The command string itself. */
//      1,                      /* Return the first parameter. */
//      &xParameterStringLength /* Store the parameter string length. */
//      );
//
//  /* Sanity check something was returned. */
//  configASSERT(pcParameter);
//
//  /* Attempt to delete the directory. */
//  iReturned = ff_rmdir(pcParameter);
//
//  if (iReturned == FF_ERR_NONE)
//  {
//    sprintf(pcWriteBuffer, "%s was deleted", pcParameter);
//  }
//  else
//  {
//    sprintf(pcWriteBuffer, "Error.  %s was not deleted", pcParameter);
//  }
//
//  strcat(pcWriteBuffer, cliNEW_LINE);
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvDELCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  const char *pcParameter;
  BaseType_t xParameterStringLength;
  FRESULT res = FR_OK;
  /* This function assumes xWriteBufferLen is large enough! */
  (void)xWriteBufferLen;
  /* Obtain the parameter string. */
  pcParameter = FreeRTOS_CLIGetParameter(
                  pcCommandString,        /* The command string itself. */
                  1,                      /* Return the first parameter. */
                  &xParameterStringLength /* Store the parameter string length. */
                );
  /* Sanity check something was returned. */
  configASSERT(pcParameter);
  /* Attempt to delete the file. */
  res = f_unlink(pcParameter);

  if (res == FR_OK) {
    sprintf(pcWriteBuffer, "%s was deleted", pcParameter);
  } else {
    sprintf(pcWriteBuffer, "Error. %s was not deleted", pcParameter);
  }

  strcat(pcWriteBuffer, cliNEW_LINE);
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvCOPYCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  char *pcSourceFile;
  const char *pcDestinationFile;
  BaseType_t xParameterStringLength;
  long lSourceLength, lDestinationLength = 0;
  FILINFO fno;
  #if _USE_LFN
  static char lfn[_MAX_LFN];
  fno.lfname = lfn;
  fno.lfsize = sizeof(lfn);
  #endif
  /* Obtain the name of the destination file. */
  pcDestinationFile = FreeRTOS_CLIGetParameter(
                        pcCommandString,        /* The command string itself. */
                        2,                      /* Return the second parameter. */
                        &xParameterStringLength /* Store the parameter string length. */
                      );
  /* Sanity check something was returned. */
  configASSERT(pcDestinationFile);
  /* Obtain the name of the source file. */
  pcSourceFile = (char *)FreeRTOS_CLIGetParameter(
                   pcCommandString,        /* The command string itself. */
                   1,                      /* Return the first parameter. */
                   &xParameterStringLength /* Store the parameter string length. */
                 );
  /* Sanity check something was returned. */
  configASSERT(pcSourceFile);
  /* Terminate the string. */
  pcSourceFile[xParameterStringLength] = 0x00;

  /* See if the source file exists, obtain its length if it does. */
  if (f_stat(pcSourceFile, &fno) == FR_OK) {
    lSourceLength = fno.fsize;
  } else {
    lSourceLength = 0;
  }

  if (lSourceLength == 0) {
    sprintf(pcWriteBuffer, "Source file does not exist");
  } else {
    /* See if the destination file exists. */
    if (f_stat(pcDestinationFile, &fno) == FR_OK) {
      lDestinationLength = fno.fsize;
    } else {
      lDestinationLength = 0;
    }

    if (fno.fattrib == AM_DIR) {
      sprintf(pcWriteBuffer, "Error: Destination is a directory not a file");
      /* Set lDestinationLength to a non-zero value just to prevent an
      attempt to copy the file. */
      lDestinationLength = 1;
    } else if (lDestinationLength != 0) {
      sprintf(pcWriteBuffer, "Error: Destination file already exists");
    }
  }

  /* Continue only if the source file exists and the destination file does
  not exist. */
  if ((lSourceLength != 0) && (lDestinationLength == 0)) {
    if (prvPerformCopy(pcSourceFile, lSourceLength, pcDestinationFile, pcWriteBuffer, xWriteBufferLen) == pdPASS) {
      sprintf(pcWriteBuffer, "Copy made");
    } else {
      sprintf(pcWriteBuffer, "Error during copy");
    }
  }

  strcat(pcWriteBuffer, cliNEW_LINE);
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvPWDCommand(char *pcWriteBuffer, size_t xWriteBufferLen, const char *pcCommandString) {
  DIR dir;
  char path[20];
  (void)xWriteBufferLen;
  (void)pcCommandString;
  /* Copy the current working directory into the output buffer. */
  f_opendir(&dir, "/html");
  f_getcwd(path, 20);
  sprintf(pcWriteBuffer, "%s", path);
  return pdFALSE;
}
/*-----------------------------------------------------------*/

static BaseType_t prvPerformCopy(const char *pcSourceFile,
                                 int32_t lSourceFileLength,
                                 const char *pcDestinationFile,
                                 char *pxWriteBuffer,
                                 size_t xWriteBufferLen) {
  BaseType_t xReturn = pdPASS;
  int32_t lBytesRead = 0, lBytesToRead, lBytesRemaining;
  UINT br, bw; //bytes var
  FIL srcFile, dstFile;

  /* NOTE:  Error handling has been omitted for clarity. */

  if ((FR_OK == f_open(&srcFile, pcSourceFile, FA_READ)) &&
      (FR_OK == f_open(&dstFile, pcDestinationFile, FA_CREATE_NEW))) {
    while (lBytesRead < lSourceFileLength) {
      /* How many bytes are left? */
      lBytesRemaining = lSourceFileLength - lBytesRead;

      /* How many bytes should be read this time around the loop.  Can't
      read more bytes than will fit into the buffer. */
      if (lBytesRemaining > (long)xWriteBufferLen) {
        lBytesToRead = (long)xWriteBufferLen;
      } else {
        lBytesToRead = lBytesRemaining;
      }

      f_read(&srcFile, pxWriteBuffer, lBytesToRead, (void *)&br);
      f_write(&dstFile, pxWriteBuffer, lBytesToRead, (void *)&bw);
      lBytesRead += lBytesToRead;
    }
  }

  f_close(&srcFile);
  f_close(&dstFile);

  if (lBytesRead == lSourceFileLength) {
    xReturn = pdPASS;
  } else {
    xReturn = pdFAIL;
  }

  return xReturn;
}
/*-----------------------------------------------------------*/

//static void prvCreateFileInfoString(char *pcBuffer, FF_FindData_t *pxFindStruct)
//{
//  const char *pcWritableFile = "writable file", *pcReadOnlyFile = "read only file", *pcDirectory = "directory";
//  const char *pcAttrib;
//
//  /* Point pcAttrib to a string that describes the file. */
//  if ((pxFindStruct->ucAttributes & FF_FAT_ATTR_DIR) != 0)
//  {
//    pcAttrib = pcDirectory;
//  }
//  else if (pxFindStruct->ucAttributes & FF_FAT_ATTR_READONLY)
//  {
//    pcAttrib = pcReadOnlyFile;
//  }
//  else
//  {
//    pcAttrib = pcWritableFile;
//  }
//
//  /* Create a string that includes the file name, the file size and the
//  attributes string. */
//  sprintf(pcBuffer, "%s [%s] [size=%d]", pxFindStruct->pcFileName, pcAttrib, (int)pxFindStruct->ulFileSize);
//}
