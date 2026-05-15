/*
 *  sensors.h
 *
 *  Created on: 1 May 2026
 *  Author: jochman03
 */

#ifndef MAIN_SENSORS_H
#define MAIN_SENSORS_H

#include <stdbool.h>
#include <stdint.h>

#define ADC_CHANNEL_COUNT     5
#define DIGITAL_CHANNEL_COUNT 5
#define SENSORS_CHANNEL_COUNT (ADC_CHANNEL_COUNT + DIGITAL_CHANNEL_COUNT)

/**
 * @brief Single measurement of all sensor values.
 */
typedef struct {
    int raw[SENSORS_CHANNEL_COUNT];
    uint32_t seq;
    int64_t timestamp_ms;
} sensors_sample_t;

/**
 * @brief Initialize ADC, GPIO and internal queue.
 *
 * Must be called before sensors_start().
 */
void sensors_init(void);

/**
 * @brief Start periodic sensor acquisition task.
 *
 * Creates FreeRTOS task that samples sensors periodically.
 */
void sensors_start(void);

/**
 * @brief Get latest available sensor sample (non-blocking).
 *
 * @param sample Output buffer for latest data.
 *
 * @return true if a new sample was available, false otherwise.
 */
bool sensors_get_latest(sensors_sample_t* sample);

#endif /* MAIN_SENSORS_H */