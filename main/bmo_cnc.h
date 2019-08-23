/*
  bmo_cnc.h
 */
#ifndef __BMO_CNC_H__
#define __BMO_CNC_H__

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_system.h"
#include "esp_event.h"
#include "esp_event_loop.h"
#include "esp_task_wdt.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "driver/gpio.h"
#include "driver/sigmadelta.h"

#include "ota_update.h"

#include "packet.h"
#include "bmo_wifi.h"

// Sets up the bmo command and control server.
void bmo_cnc_init();

#endif // __NETWORK_H__
