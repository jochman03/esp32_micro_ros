#ifndef MAIN_ADC_H
#define MAIN_ADC_H

#include <stdbool.h>
#include <stdint.h>

#define ADC_CHANNEL_COUNT 6

typedef struct {
    int raw[ADC_CHANNEL_COUNT];
    uint32_t seq;
    int64_t timestamp_ms;
} adc_sample_t;

void adc_init(void);
void adc_start(void);
bool adc_get_latest(adc_sample_t *sample);

#endif // MAIN_ADC_H