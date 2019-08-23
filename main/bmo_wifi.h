#ifndef __BMO_WIFI_H__
#define __BMO_WIFI_H__

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"

void bmo_wifi_init();
bool bmo_wifi_connected();
void bmo_wifi_connect();

#endif
