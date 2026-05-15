#include <stdbool.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "nvs_flash.h"

#include "adc.h"
#include "espnow_tx.h"

#define BUTTON_GPIO    GPIO_NUM_25
#define ESPNOW_CHANNEL 3

static void wifi_init(void);
static void button_init(void);
static void espnow_task(void* args);

static const char* TAG = "MAIN";

void app_main(void) {
    wifi_init();
    adc_init();
    button_init();
    espnow_init();

    xTaskCreate(espnow_task, "espnow_task", 4096, NULL, 5, NULL);
}

static void wifi_init(void) {
    esp_err_t ret = nvs_flash_init();

    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    } else {
        ESP_ERROR_CHECK(ret);
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_set_channel(ESPNOW_CHANNEL, WIFI_SECOND_CHAN_NONE));

    ESP_LOGI(TAG, "Wi-Fi started for ESP-NOW, channel=%d", ESPNOW_CHANNEL);
}

static void button_init(void) {
    gpio_config_t cfg = {
        .pin_bit_mask = 1ULL << BUTTON_GPIO,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&cfg));
}

static void espnow_task(void* args) {
    uint32_t seq = 0;

    while (1) {
        espnow_packet_t packet = {0};

        packet.seq = seq++;
        packet.adc[0] = adc_read_x();
        packet.adc[1] = adc_read_y();
        packet.battery_mv = adc_read_battery();
        packet.button = gpio_get_level(BUTTON_GPIO) == 0;

        espnow_send(&packet);

        ESP_LOGI(TAG, "TX seq=%lu adc0=%d adc1=%d button=%d", (unsigned long)packet.seq,
                 packet.adc[0], packet.adc[1], packet.button);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}