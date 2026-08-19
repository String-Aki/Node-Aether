#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

extern EventGroupHandle_t wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0

void wifi_init(void);

#endif