/*
 *  esp_now_rx.h
 *
 *  Created on: 1 May 2026
 *  Author: jochman03
 */

#ifndef MAIN_ESP_NOW_H
#define MAIN_ESP_NOW_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ESPNOW_ADC_COUNT 2

/**
 * @brief ESPNOW data packet structure.
 */
typedef struct __attribute__((packed)) {
    uint32_t seq;
    int16_t adc[ESPNOW_ADC_COUNT];
    int16_t battery_mv;
    bool button;
} espnow_packet_t;

/**
 * @brief Initialize ESPNOW communication.
 */
void espnow_init(void);

/**
 * @brief Get latest received ESPNOW packet.
 *
 * @param packet Pointer to output packet structure.
 * @return true if received correct packet, false otherwise.
 */
bool espnow_get_latest(espnow_packet_t* packet);

#ifdef __cplusplus
}
#endif

#endif /* MAIN_ESP_NOW_H */