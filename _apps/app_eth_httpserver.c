/**
  ******************************************************************************
  * @file    app_usdpsyslog.c
  * @author  PavelB
  * @version V1.0
  * @date    25-March-2017
  * @brief   udp syslog send task
  ******************************************************************************
  */


/* Includes ------------------------------------------------------------------*/
#include "BSP_IZUMRUD.h"
#include "cmsis_os.h"
#include "lcd_log.h"

/* lwIP includes. */
#include "lwip/api.h"
#include "httpd_structs.h"

/* STM32 includes. */
#include "fatfs.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static uint8_t sector[4096];
static uint32_t loginOK = 0;

/* Private function prototypes -----------------------------------------------*/
extern uint16_t StorageFindFile(char *);

void StaticWebPage(struct netconn *conn, char *page, uint32_t code);
void DynamicPartOfWebPage(struct netconn *conn);

/*******************************************************************************
  * Task: Web-server
  * 1.
  * 2.
  * 3.
  * ...
  *****************************************************************************/
static void prvHTTPServerTask(void const *pvParameters) {
  static struct netconn *conn, *newconn;
  static struct netbuf *netbuf;
  err_t err;
  char *buf;
  u16_t buflen;
  char method[5], request[32], page[32], user[10], pass[10];

  /* Create a new TCP connection handle */
  /* Create a new connection identifier. */
  if (NULL != (conn = netconn_new(NETCONN_TCP))) {
    /* Bind to port 80 (HTTP) with default IP address */
    if (ERR_OK == netconn_bind(conn, NULL, 80)) {
      /* Put the connection into LISTEN state */
      netconn_listen(conn);

      for (;;) {
        /* Wait for an incoming connection and Process the new connection. */
        if (ERR_OK == netconn_accept(conn, &newconn)) {
          /* Read the data from the port, blocking if nothing yet there.
           We assume the request (the part we care about) is in one netbuf */
          err = netconn_recv(newconn, &netbuf);

          if (err == ERR_OK) {
            err = netbuf_data(netbuf, (void**)&buf, &buflen);

            /* Is this an HTTP GET command? (only check the first 5 chars, since
            there are other formats for GET, and we're keeping it very simple )*/
            if (buflen >= 5) {
              sscanf(buf, "%s /%s", method, request);

              if (strstr(method, "GET")) {
                if (strstr(request, "HTTP/1.1"))  {
                  StaticWebPage(newconn, "www/login.html", HTTP_HDR_OK_11);
                } else  {
                  memset(page, 0, 32);
                  strcat(page, "www/");
                  strcat(page, request);

                  if (StorageFindFile(page)) {
                    StaticWebPage(newconn, page, HTTP_HDR_OK_11);

                    // is this "tasklist.html" request ???
                    if (strstr(page, "tasklist.html")) {
                      DynamicPartOfWebPage(newconn);
                    }
                  } else {
                    StaticWebPage(newconn, "www/404.html", HTTP_HDR_NOT_FOUND_11);
                  }
                }
              } else if (strstr(method, "POST")) {
                loginOK = 0;

                if (strstr(request, "login.cgi")) {
                  memset(user, 0, 10);
                  memset(pass, 0, 10);

                  /* parse packet for the username & password */
                  for (uint32_t i = 0; i < buflen; i++) {
                    if (strncmp((buf + i), "username=", 9) == 0) {
                      i += 9; // skeep 9 bytes of "username="

                      for (uint32_t j = 0; j < 10 && *(buf + i) != '&'; j++, i++) {
                        user[j] = *(buf + i);
                      }

                      i += 10; // skeep 10 bytes of '&password='

                      for (uint32_t j = 0; j < 10 && *(buf + i) != ' ' && *(buf + i) != '\0'; j++, i++) {
                        pass[j] = *(buf + i);
                      }

                      // Check users from preBuild list
                      // USER1
                      if (strstr(user, USER1ID) && strstr(pass, PASSWORD1)) loginOK = 1;

                      // USER2
                      if (strstr(user, USER2ID) && strstr(pass, PASSWORD2)) loginOK = 1;

                      break;
                    }
                  }

                  if (loginOK) {
                    LCD_DbgLog("WEB | %s\nWEB | %s\n", user, pass);
                  } else {
                    LCD_DbgLog("WEB | неверный пароль!\n");
                  }
                } else if (strstr(request, "logout.cgi")) {
                  LCD_DbgLog("-=* Logout *=-\n");
                }

                // what page send to client?
                if (loginOK) {
                  StaticWebPage(newconn, "www/index.html", HTTP_HDR_OK_11);
                } else {
                  StaticWebPage(newconn, "www/login.html", HTTP_HDR_OK_11);
                }
              }
            }
          }

          /* Close the connection (server closes in HTTP) */
          netconn_close(newconn);
          /* Delete the buffer (netconn_recv gives us ownership,
           so we have to make sure to deallocate the buffer) */
          netbuf_delete(netbuf);
          /* delete connection */
          netconn_delete(newconn);
        }
      }
    }
  }
}

/*----------------------------------------------------------------------------*/
void vStartHTTPServerTask() {
  /* Create that task */
  osThreadDef(HTTPServer,
              prvHTTPServerTask,
              osPriorityHigh,
              0,
              2048);
  osThreadCreate(osThread(HTTPServer), NULL);
  BSP_AUDIO_Play(SND_HTTPSERVERUP);
  LCD_UsrLog("WEB | сервер запущен!\n");
  stringCatAndView("LAN | start HTTP Server!");
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Send a static Web Page from SD card.
  * @param  conn pointer on connection structure, fileName of 'page'
  * @retval None
  */
void StaticWebPage(struct netconn *conn, char *page, uint32_t ind) {
  uint32_t i, j;
  FRESULT res;
  FIL F1;
  UINT nBytes = 0;
  char *PAGE_BODY;
  char ext[5] = {0};
  static uint32_t errCounter = 0;
  // check extention -> first of all search '.'
  i = 0;

  while (*(page + i) != '.') {
    ++i;
  }

  ++i;
  j = 0;

  // save extention of F1
  while (*(page + i) != '\0') {
    ext[j] = *(page + i);
    ++i;
    ++j;
  }

  // if no login and .html -> redirect to login page
  if (loginOK == 0 && strstr(ext, "html")) {
    sprintf(page, "www/login.html");
  }

  // open page file on SD card
  if (FR_OK != (res = f_open(&F1, page, FA_OPEN_EXISTING | FA_READ))) {
    LCD_ErrLog("#%03d | Can't open '%s' (err %d)\n", ++errCounter, page, res);
    osDelay(10);
    return;
  }

  PAGE_BODY = (char *)sector;
  memset(PAGE_BODY, 0, 4096);
  //////////////////////////////////////////////////////////////////////////////
  // header of HTTP
  strcat(PAGE_BODY, g_psHTTPHeaderStrings[ind]);
  strcat(PAGE_BODY, "Server: lwIP/1.3.1 (http://savannah.nongnu.org/projects/lwip)\r\n");

  /* A list of extension-to-HTTP header strings */
  for (i = 0; i < NUM_HTTP_HEADERS; i++) {
    if (strstr(ext, g_psHTTPHeaders[i].extension)) {
      strcat(PAGE_BODY, g_psHTTPHeaders[i].content_type);
      break;
    }
  }

  netconn_write(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);

  //////////////////////////////////////////////////////////////////////////////
  // body
  // read file with 4096 blocks
  while (f_tell(&F1) < f_size(&F1)) {
    if (FR_OK != (res = f_read(&F1, PAGE_BODY, 4096, &nBytes))) {
      LCD_ErrLog("#%03d | Can't read '%s' (err %d)\n", ++errCounter, page, res);
      osDelay(10);
      break;
    } else {
      netconn_write(conn, PAGE_BODY, nBytes, NETCONN_COPY);
    }
  }

  // close request
  if (FR_OK != (res = f_close(&F1))) {
    LCD_ErrLog("#%03d | Can't close F1 (err %d)\n", ++errCounter, res);
  }
}

/**
  * @brief  Create and send a dynamic Web Page. This page contains the list of
  *         running tasks and the number of page hits.
  * @param  conn pointer on connection structure
  * @retval None
  */
void DynamicPartOfWebPage(struct netconn *conn) {
  static uint8_t tID = 0;
  char *PAGE_BODY = (char *)sector;
  memset(PAGE_BODY, 0, 4096);

  /* Update the hit count */
  for (uint32_t i = 255; i; i--) {
    sprintf(PAGE_BODY, "<tr><td>2017/05/18 14:17:07</td><td>Ping...</td><td>192.000.046.102</td><td>0</td><td>0</td><td>0</td><td>0</td><td>1</td><td>%u</td><td>-79</td><td>49</td><td>+30.0</td><td>3317</td><td>0</td><td>0</td></tr>",
            tID++);
    /* Send the dynamically generated page */
    netconn_write(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
  }

  sprintf(PAGE_BODY, "</table></main></body></html>");
  /* Send the dynamically generated page */
  netconn_write(conn, PAGE_BODY, strlen(PAGE_BODY), NETCONN_COPY);
}
