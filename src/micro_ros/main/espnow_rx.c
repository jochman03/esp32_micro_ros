
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"

#include "espnow_rx.h"

static const char* TAG = "ESP_NOW";

static QueueHandle_t espnow_queue = NULL;

static void espnow_print_mac(void);
static void espnow_callback(const esp_now_recv_info_t* recv_info, const uint8_t* data,
                            int data_len);

void espnow_init(void) {
    espnow_queue = xQueueCreate(1, sizeof(espnow_packet_t));
    if (espnow_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create ESP-NOW RX queue");
        return;
    }

    espnow_print_mac();

    esp_err_t err = esp_now_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_now_init failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_ERROR_CHECK(esp_now_register_recv_cb(espnow_callback));

    ESP_LOGI(TAG, "ESP-NOW ready");
}

bool espnow_get_latest(espnow_packet_t* packet) {
    if (packet == NULL || espnow_queue == NULL) {
        return false;
    }

    return xQueueReceive(espnow_queue, packet, 0) == pdTRUE;
}

static void espnow_print_mac(void) {
    uint8_t mac[6] = {0};

    esp_err_t err = esp_wifi_get_mac(WIFI_IF_STA, mac);
    if (err != ESP_OK) {
        ESP_LOGW(TAG, "Failed to get STA MAC: %s", esp_err_to_name(err));
        return;
    }

    ESP_LOGI(TAG, "STA MAC: %02X:%02X:%02X:%02X:%02X:%02X", mac[0], mac[1], mac[2], mac[3], mac[4],
             mac[5]);
}

static void espnow_callback(const esp_now_recv_info_t* recv_info, const uint8_t* data,
                            int data_len) {
    if (recv_info == NULL || data == NULL) {
        return;
    }

    if (data_len != sizeof(espnow_packet_t)) {
        ESP_LOGW(TAG, "Wrong packet size: %d, expected: %u", data_len,
                 (unsigned int)sizeof(espnow_packet_t));
        return;
    }

    espnow_packet_t packet;
    memcpy(&packet, data, sizeof(packet));

    if (espnow_queue != NULL) {
        xQueueOverwrite(espnow_queue, &packet);
    }

    ESP_LOGI(TAG, "RX from %02X:%02X:%02X:%02X:%02X:%02X seq=%lu x=%d y=%d bat=%d button=%d",
             recv_info->src_addr[0], recv_info->src_addr[1], recv_info->src_addr[2],
             recv_info->src_addr[3], recv_info->src_addr[4], recv_info->src_addr[5],
             (unsigned long)packet.seq, packet.adc[0], packet.adc[1], packet.battery_mv,
             packet.button);
}