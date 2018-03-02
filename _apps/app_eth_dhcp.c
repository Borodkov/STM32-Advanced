/**
 ******************************************************************************
  * File Name          : app_eth_dhcp.c
  * Description        : This file provides initialization code for LWIP middleWare.
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

#include "lwip/opt.h"
#include "lwip/mem.h"
#include "lwip/memp.h"
#include "lwip/dhcp.h"
#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "lwip/netif.h"

#include "netif/etharp.h"
#include "ethernetif.h"

#include <string.h>
#include "lcd_log.h"

/* Private define ------------------------------------------------------------*/
/*Static IP ADDRESS*/
#define IP_ADDR0   192
#define IP_ADDR1   168
#define IP_ADDR2   0
#define IP_ADDR3   10

/*NETMASK*/
#define NETMASK_ADDR0   255
#define NETMASK_ADDR1   255
#define NETMASK_ADDR2   255
#define NETMASK_ADDR3   0

/*Gateway Address*/
#define GW_ADDR0   192
#define GW_ADDR1   168
#define GW_ADDR2   0
#define GW_ADDR3   1

#define MAX_DHCP_TRIES  3

#define DHCP_OFF                   (uint8_t) 0
#define DHCP_START                 (uint8_t) 1
#define DHCP_WAIT_ADDRESS          (uint8_t) 2
#define DHCP_ADDRESS_ASSIGNED      (uint8_t) 3
#define DHCP_TIMEOUT               (uint8_t) 4
#define DHCP_LINK_DOWN             (uint8_t) 5

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
volatile uint8_t DHCP_state = DHCP_OFF;

/* Variables Initialization */
struct netif gnetif;
ip4_addr_t ipaddr;
ip4_addr_t netmask;
ip4_addr_t gw;

/* Private function prototypes -----------------------------------------------*/
extern void vStartTCPCommandInterpreterTask(void);
extern void vStartHTTPServerTask(void);

/**
  * @brief  Notify the User about the network interface config status
  * @param  netif: the network interface
  * @retval None
  */
void User_notification(struct netif *netif) {
  if (netif_is_up(netif)) {
    DHCP_state = DHCP_START;
    LCD_UsrLog("LAN | Wait DHCP...\n");
  } else {
    DHCP_state = DHCP_LINK_DOWN;
    LCD_UsrLog("LAN | no LAN cable\n");
  }
}

/**
  * LwIP initialization function
  */
void MX_LWIP_Init(void) {
  /* Initilialize the LwIP stack with RTOS */
  tcpip_init(NULL, NULL);
  /* IP addresses initialization with DHCP (IPv4) */
  ipaddr.addr = 0;
  netmask.addr = 0;
  gw.addr = 0;
  /* add the network interface (IPv4/IPv6) with RTOS */
  netif_add(&gnetif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);
  /* Registers the default network interface */
  netif_set_default(&gnetif);

  if (netif_is_link_up(&gnetif)) {
    /* When the netif is fully configured this function must be called */
    netif_set_up(&gnetif);
  } else {
    /* When the netif link is down this function must be called */
    netif_set_down(&gnetif);
  }

  /* Start DHCP negotiation for a network interface (IPv4) */
  dhcp_start(&gnetif);
  /* Notify user about the network interface config */
  User_notification(&gnetif);
}

/*******************************************************************************
  * Task: DHCP Process, argument: network interface
  * 1. Link up
  * 2. Get IP from DHCP
  * 3. Open socket for Telnet on "23" port
  * ...
  *****************************************************************************/
static void task(void const *pvParameters) {
  struct netif *netif = (struct netif *) pvParameters;
  struct dhcp *dhcp;

  for (;;) {
    switch (DHCP_state) {
      case DHCP_START: {
          ip_addr_set_zero_ip4(&netif->ip_addr);
          ip_addr_set_zero_ip4(&netif->netmask);
          ip_addr_set_zero_ip4(&netif->gw);
          DHCP_state = DHCP_WAIT_ADDRESS;
        }
        break;

      case DHCP_WAIT_ADDRESS: {
          if (dhcp_supplied_address(netif)) {
            DHCP_state = DHCP_ADDRESS_ASSIGNED;
            /* TCP CLI */
            vStartTCPCommandInterpreterTask();
            LCD_UsrLog("LAN | open Telnet socket %s:23\n", ip4addr_ntoa((const ip4_addr_t *) &netif->ip_addr));
//        /* HTTP Server */
//        vStartHTTPServerTask();
//
//        BSP_AUDIO_Play(SND_HTTPSERVERUP);
//
//        LCD_UsrLog("LAN | запущен сервер\n");
//        LCD_UsrLog("LAN | HTTP: \n");
//        LCD_UsrLog("LAN | %s\n", ip4addr_ntoa((const ip4_addr_t *)&netif->ip_addr));
          } else {
            dhcp = (struct dhcp *) netif_get_client_data(netif, LWIP_NETIF_CLIENT_DATA_INDEX_DHCP);

            /* DHCP timeout */
            if (dhcp->tries > MAX_DHCP_TRIES) {
              DHCP_state = DHCP_TIMEOUT;
              /* Stop DHCP */
              dhcp_stop(netif);
              /* Static address used */
              IP_ADDR4(&ipaddr, IP_ADDR0, IP_ADDR1, IP_ADDR2, IP_ADDR3);
              IP_ADDR4(&netmask, NETMASK_ADDR0, NETMASK_ADDR1, NETMASK_ADDR2, NETMASK_ADDR3);
              IP_ADDR4(&gw, GW_ADDR0, GW_ADDR1, GW_ADDR2, GW_ADDR3);
              netif_set_addr(netif, ip_2_ip4(&ipaddr), ip_2_ip4(&netmask), ip_2_ip4(&gw));
              LCD_UsrLog("LAN | DHCP Timeout. Static IP: %s\n", ip4addr_ntoa((const ip4_addr_t *) &netif->ip_addr));
            }
          }
        }
        break;

      case DHCP_LINK_DOWN: {
          /* Stop DHCP */
          dhcp_stop(netif);
          DHCP_state = DHCP_OFF;
        }
        break;

      case DHCP_ADDRESS_ASSIGNED: {
          //osThreadSuspend(NULL);
          osThreadTerminate(NULL);
        }
        break;

      default:
        break;
    }

    osDelay(100);
  }
}

/*----------------------------------------------------------------------------*/
void vStartDHCPTask() {
  LCD_UsrLog("vStartDHCPTask()\n");
  MX_LWIP_Init();
  osThreadDef(DHCP,
              task,
              osPriorityNormal,
              0,
              configMINIMAL_STACK_SIZE);
  osThreadCreate(osThread(DHCP), &gnetif);
}
