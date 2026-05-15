/*
 *  adc.h
 *
 *  Created on: 2 May 2026
 *  Author: jochman03
 */

#ifndef MAIN_ADC_H
#define MAIN_ADC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize ADC peripherals and channels.
 */
void adc_init(void);

/**
 * @brief Read X-axis analog value.
 *
 * @return X-axis ADC value.
 */
int16_t adc_read_x(void);

/**
 * @brief Read Y-axis analog value.
 *
 * @return Y-axis ADC value.
 */
int16_t adc_read_y(void);

/**
 * @brief Read battery voltage ADC value.
 *
 * @return Battery voltage in millivolts.
 */
int16_t adc_read_battery(void);

#ifdef __cplusplus
}
#endif

#endif // MAIN_ADC_H