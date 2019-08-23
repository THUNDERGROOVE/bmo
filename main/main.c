/*
  Distributed Lighting Solution
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "bmo_wifi.h"

#include "bmo_cnc.h"
#include "packet.h"

#define TAG "bmo-main"

void app_main(void) {
    ESP_LOGI(TAG, "packet size: %d", sizeof(Packet));
    ESP_LOGI(TAG, "bmo starting up!");
    nvs_flash_init();

    bmo_wifi_init();

    bmo_wifi_connect();

    bmo_cnc_init();

}
