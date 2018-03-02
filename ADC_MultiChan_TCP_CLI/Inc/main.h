/**
  ******************************************************************************
  * File Name          : main.h
  * Description        : This file contains the common defines of the application
  ******************************************************************************
  */

#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>
#include <stdio.h>

#include "board.h"

/* FreeRTOS+CLI definitions. */
/* Dimensions a buffer into which command outputs can be written. The buffer
 * can be declared in the CLI code itself, to allow multiple command consoles to
 * share the same buffer. For example, an application may allow access to the
 * command interpreter by UART and by Ethernet. Sharing a buffer is done purely
 * to save RAM. Note, however, that the command console itself is not re-entrant,
 * so only one command interpreter interface can be used at any one time. For
 * that reason, no attempt at providing mutual exclusion to the buffer is
 * attempted.
 */
#define configCOMMAND_INT_MAX_OUTPUT_SIZE	512

/* Dimensions the buffer into which input characters are placed. */
#define cmdMAX_INPUT_SIZE 60

/* DEL acts as a backspace. */
#define cmdASCII_DEL (0x7F)
 
/* Const messages output by the command console. */
#define pcWelcomeMessage     "FreeRTOS CLI server.\r\nEnter 'help' to view a list of available commands.\r\n\r\n>>\r\n"
#define pcEndOfOutputMessage "\r\n[Press ENTER to execute the previous command again]\r\n>>\r\n"
#define pcNewLine            "********************************************************************************\r\n"

/*   */
#define ENCODER_PB	( 1 << 0 ) // push button
#define ENCODER_FW	( 1 << 1 ) // forward rotation
#define ENCODER_BW	( 1 << 2 ) // backward rotation

void _Error_Handler(char *, int);

#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

#endif /* __MAIN_H */
