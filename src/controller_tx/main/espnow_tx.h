/*
 *  espnow_tx.h
 *
 *  Created on: 2 May 2026
 *  Author: jochman03
 */

#ifndef MAIN_ESPNOW_TX_H
#define MAIN_ESPNOW_TX_H

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Number of joystick ADC channels
 */
#define ESPNOW_ADC_COUNT 2

/**
 * @brief ESPNOW data packet.
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
 * @brief Send ESPNOW packet.
 *
 * @param packet Pointer to packet data.
 */
void espnow_send(const espnow_packet_t* packet);

#ifdef __cplusplus
}
#endif

#endif // MAIN_ESPNOW_TX_H