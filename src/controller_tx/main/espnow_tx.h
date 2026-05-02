#ifndef MAIN_ESPNOW_TX_H
#define MAIN_ESPNOW_TX_H

#include <stdbool.h>
#include <stdint.h>

#define ESPNOW_ADC_COUNT 2

typedef struct __attribute__((packed)) {
    uint32_t seq;
    int16_t adc[ESPNOW_ADC_COUNT];
    int16_t battery_mv;
    bool button;
} espnow_packet_t;

void espnow_init(void);
void espnow_send(const espnow_packet_t *packet);


#endif // MAIN_ESPNOW_TX_H