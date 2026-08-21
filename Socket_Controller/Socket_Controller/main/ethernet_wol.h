#ifndef ETHERNET_WOL_H
#define ETHERNET_WOL_H

#include "esp_eth.h"

extern esp_eth_handle_t eth_handle;

void init_w5500_ethernet(void);
void send_wake_on_lan(void);
void wol_button_task(void *arg);

#endif