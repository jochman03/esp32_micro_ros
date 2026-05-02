#ifndef MAIN_ADC_H
#define MAIN_ADC_H

#include <stdint.h>

void adc_init(void);
int16_t adc_read_x(void);
int16_t adc_read_y(void);
int16_t adc_read_battery(void);

#endif //MAIN_ADC_H