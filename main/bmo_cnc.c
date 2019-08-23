/*
  bmo_cnc.c
  command & control service.  Use with vmo tool for OTA over tcpip & more.
*/

#include "bmo_cnc.h"

#define TAG "bmo_cnc"

static EventGroupHandle_t seg;

// Indicates that we should trigger a re-boot after sending the response.
static int otamode = 0;

// Listen to TCP requests on port 80
static void bmo_cnc_task(void *pvParameters);

static void process_packet(int sock, Packet *p);

void bmo_cnc_init() {
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    ESP_LOGI(TAG, "bmo_cnc_init");
    
    seg = xEventGroupCreate();
    xTaskCreate(&bmo_cnc_task, "bmo_cnc_task", 32768, NULL, 5, NULL);
    
    otaUpdateInit();
}

static void bmo_cnc_task(void *pvParameters) {
    const int tcpPort = 80;
    
    ESP_LOGI(TAG, "bmo_cnc_task");
    while (!bmo_wifi_connected()){
        ESP_LOGI(TAG, "Waiting for WiFi connection");
        vTaskDelay(1000/ portTICK_RATE_MS);
    }
    
    while (1) {
        // Barrier for the connection (we need to be connected to an AP).
        
        
        // Create TCP socket.
        
        int s = socket(AF_INET, SOCK_STREAM, 0);
        if (s < 0) {
            ESP_LOGE(TAG, "bmo_cnc_task: failed to create socket: %d (%s)", errno, strerror(errno));
            vTaskDelay(1000 / portTICK_RATE_MS);
            continue;
        }
        
        
        // Bind socket to port.
        
        struct sockaddr_in serverAddr;
        memset(&serverAddr, 0, sizeof (struct sockaddr_in));
        serverAddr.sin_len = sizeof(struct sockaddr_in);
        serverAddr.sin_family = AF_INET;
        serverAddr.sin_port = htons(tcpPort);
        serverAddr.sin_addr.s_addr = INADDR_ANY;
        
        int b = bind(s, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in));
        if (b < 0) {
            ESP_LOGE(TAG, "bmo_cnc_task: failed to bind socket %d: %d (%s)", s, errno, strerror(errno));
            vTaskDelay(1000 / portTICK_RATE_MS);
            continue;
        }
        
        
        // Listen to incoming connections.
        
        ESP_LOGD(TAG, "bmo_cnc_task: 'listen' on socket %d", s);
        listen(s, 1); // backlog max. 1 connection
        while (1) {
            
            
            // Accept the connection on a separate socket.
            
            ESP_LOGI(TAG, "--------------------");
            ESP_LOGI(TAG, "bmo_cnc_task: 'accept' on socket %d", s);
            struct sockaddr_in clientAddr;
            socklen_t clen = sizeof(clientAddr);
            int s2 = accept(s, (struct sockaddr *)&clientAddr, &clen);
            if (s2 < 0) {
                ESP_LOGE(TAG, "bmo_cnc_task: 'accept' failed: %d (%s)", errno, strerror(errno));
                vTaskDelay(1000 / portTICK_RATE_MS);
                break;
            }
            
            Packet *p = (Packet *)malloc(sizeof(Packet));
            do {
                bzero(p, sizeof(Packet));
                int n = ReadPacket(s2, p);
                ESP_LOGI(TAG, "read %d bytes", n);
                if (n <= 0) {
                    ESP_LOGI(TAG, "bmo_cnc_task: ReadPacket -> %d closing socket", n);
                    close(s2);
                    break;
                }
                process_packet(s2, p);
            } while (1);
        }
        
        // Should never arrive here
        close(s);
        vTaskDelay(2000 / portTICK_RATE_MS);
    }
}

static void process_packet(int sock, Packet *p) {
    Packet out = {0};
    TOtaResult result = OTA_OK;
    ESP_LOGI(TAG, "incoming packet: %d %s\n", p->type, PacketTypeString(p->type));
    switch (p->type) {
    case PacketOTAEnd: {
        ESP_LOGI(TAG, "Process Packet: OTA mode deactivated.");
        otamode = false;
        out.type = PacketOK;
        result = otaUpdateEnd();
    } break;
    case PacketOTAData: {
        result = otaUpdateWriteHexData(p->ota_data.chunk,
                                       p->ota_data.size);
        out.type = PacketOK;
    } break;
    case PacketPing: {
        // @TODO
    } break;
    case PacketReboot: {
        ESP_LOGI(TAG, "Process Packet: Reboot");
        esp_restart();
    } break;
    case PacketCMD: {
        // @TODO
    } break;
    case PacketEnterOTA: {
            ESP_LOGI(TAG, "Process Packet: OTA mode activated.");
            result = otaUpdateBegin();
            out.type = PacketOK;
            WritePacket(sock, &out);
            otamode = true;
        } break;
    default:
        {
        } break;
    }
    if (result != OTA_OK) {
        ESP_LOGE(TAG, "Process Packet: OTA_ERROR %d", result);
        //sprintf(response, "OTA_ERROR %d\r\n", result);
        out.type = PacketFail;
        WritePacket(sock, &out);
    }
}
