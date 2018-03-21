/**
  ******************************************************************************
    @file    app_usdpsyslog.c
    @author  PavelB
    @version V1.0
    @date    25-March-2017
    @brief   udp syslog send task
  ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "cmsis_os.h"
#include "lcd_log.h"

#include "lwip/udp.h"
#include "lwip/api.h"

/* Private typedef -----------------------------------------------------------*/
typedef struct {
    ip_addr_t ip;
    u16_t port;
} syslogClient_t;
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osThreadId UDPSysLogTaskHandle;
osSemaphoreId xBinSem_UDPSysLogHandle = NULL;
syslogClient_t syslogClients[10] = {0};
static char s[256] = {0};
/* Private function prototypes -----------------------------------------------*/
extern void stringCatAndView(char *str);
extern char strToView[100];

static void task(void const *pvParameters) {
//  char tag[10] = {0};
//  uint8_t priority;
    static struct udp_pcb *UDPSock;
    UDPSock = udp_new();
    udp_recv(UDPSock, NULL, NULL);
    udp_bind(UDPSock, IP_ADDR_ANY, 514);
    udp_connect(UDPSock, IP_ADDR_ANY, 514);

    for (;;) {
        osSemaphoreWait(xBinSem_UDPSysLogHandle, osWaitForever); // w8 semaphore forever
        struct pbuf *p;
        uint16_t len = strlen(strToView);

        for (uint32_t i = 0; i < 10; i++) {
            if (syslogClients[i].port != 0) {
                p = pbuf_alloc(PBUF_TRANSPORT, len, PBUF_RAM);

                if (p != NULL) {
                    memcpy(p->payload, strToView, len);
                    udp_sendto(UDPSock, p, &syslogClients[i].ip, syslogClients[i].port);
                    pbuf_free(p);
                }
            }
        }
    }
}

/*----------------------------------------------------------------------------*/
void vStartUDPSysLogTask(const char *sServerIP) {
    /* definition and creation of xBinSem_uSD */
    if (xBinSem_UDPSysLogHandle == NULL) {
        osSemaphoreDef(xBinSem_UDPSysLog);
        xBinSem_UDPSysLogHandle = osSemaphoreCreate(osSemaphore(xBinSem_UDPSysLog), 1);
        osSemaphoreWait(xBinSem_UDPSysLogHandle, 0);
    }

    if (UDPSysLogTaskHandle == NULL) {
        osThreadDef(UDPSYSLOG,
                    task,
                    osPriorityBelowNormal,
                    0,
                    DEFAULT_THREAD_STACKSIZE);
        UDPSysLogTaskHandle = osThreadCreate(osThread(UDPSYSLOG), NULL);
    }

    unsigned int ip1, ip2, ip3, ip4, port;
    // read and set server ip addr from pcTaskParam
    sscanf(sServerIP, "%u.%u.%u.%u:%u", &ip1, &ip2, &ip3, &ip4, &port);

    // FIFO of syslogClients
    for (uint32_t i = 10 - 1; i > 0; i--) {
        syslogClients[i].ip   = syslogClients[i - 1].ip;
        syslogClients[i].port = syslogClients[i - 1].port;
    }

    IP4_ADDR(&syslogClients[0].ip, ip1, ip2, ip3, ip4);
    syslogClients[0].port = port;
//  BSP_AUDIO_Play(SND_SYSLOGUP);
    LCD_UsrLog("LAN | запущен сервер\n");
    LCD_UsrLog("LAN | UDP SYSLOG IP\n");
    LCD_UsrLog("LAN | %s:%u\n", ip4addr_ntoa((const ip4_addr_t *)&syslogClients[0].ip), syslogClients[0].port);
    sprintf(s, "LAN | UDP SYSLOG Server <%s:%u> have been started!",
            ip4addr_ntoa((const ip4_addr_t *)&syslogClients[0].ip), syslogClients[0].port);
    stringCatAndView(s);
}
