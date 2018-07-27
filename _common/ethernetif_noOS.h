/*******************************************************************************
    Description: This file provides initialization code for LWIP middleWare.
*******************************************************************************/

#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__

#include "lwip/err.h"
#include "lwip/netif.h"

/* Exported functions ------------------------------------------------------- */
err_t ethernetif_init(struct netif *netif);
void ethernetif_input(struct netif *netif);
void ethernetif_notify_conn_changed(struct netif *netif);
#endif
