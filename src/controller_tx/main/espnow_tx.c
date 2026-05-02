#include "espnow_tx.h"

#include <string.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_now.h"
#include "esp_wifi.h"

#define ESPNOW_CHANNEL 3

static const char* TAG = "ESP_NOW";

static const uint8_t broadcast_mac[6] = {
    0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

static void espnow_send_callback(const uint8_t* mac_addr, esp_now_send_status_t status){
    ESP_LOGI(TAG, "Send status: %s",
        status == ESP_NOW_SEND_SUCCESS ? "OK" : "FAIL");
}

void espnow_init(void){
    esp_err_t err = esp_now_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_now_init failed: %s", esp_err_to_name(err));
        return;
    }

    ESP_ERROR_CHECK(esp_now_register_send_cb(espnow_send_callback));

    esp_now_peer_info_t peer = {0};

    memcpy(peer.peer_addr, broadcast_mac, 6);
    peer.channel = ESPNOW_CHANNEL;
    peer.ifidx = WIFI_IF_STA;
    peer.encrypt = false;

    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    ESP_LOGI(TAG, "ESP-NOW TX ready");
}

void espnow_send(const espnow_packet_t* packet){
    if (packet == NULL) {
        return;
    }

    esp_err_t err = esp_now_send(broadcast_mac, (const uint8_t *)packet, sizeof(espnow_packet_t));
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_now_send failed: %s", esp_err_to_name(err));
    }
}