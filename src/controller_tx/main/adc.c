#include <stdbool.h>

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_err.h"

#include "adc.h"
#include "utils.h"


#define ADC_X_CHANNEL ADC_CHANNEL_6
#define ADC_Y_CHANNEL ADC_CHANNEL_7
#define ADC_BAT_CHANNEL ADC_CHANNEL_4

#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_MAX_12BIT 4095

static adc_oneshot_unit_handle_t adc_handle;
static adc_cali_handle_t adc_cali_handle = NULL;
static int16_t adc_read_channel_mv(adc_channel_t channel);
static int16_t adc_read_channel_raw(adc_channel_t channel);
static bool adc_calibration_init(void);

static bool adc_calibrated = false;

void adc_init(void){
    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_X_CHANNEL, &chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_Y_CHANNEL, &chan_cfg));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, ADC_BAT_CHANNEL, &chan_cfg));

    adc_calibrated = adc_calibration_init();
}

int16_t adc_read_x(void){
    int16_t x_mv = adc_read_channel_mv(ADC_X_CHANNEL);
    int16_t bat_mv = adc_read_battery();

    if (x_mv < 0 || bat_mv <= 0) {
        return -1;
    }

    int32_t value = ((int32_t)x_mv * ADC_MAX_12BIT) / bat_mv;

    return clamp_int16(value, 0, ADC_MAX_12BIT);
}

int16_t adc_read_y(void){
    int16_t y_mv = adc_read_channel_mv(ADC_Y_CHANNEL);
    int16_t bat_mv = adc_read_battery();

    if (y_mv < 0 || bat_mv <= 0) {
        return -1;
    }

    int32_t value = ((int32_t)y_mv * ADC_MAX_12BIT) / bat_mv;

    return clamp_int16(value, 0, ADC_MAX_12BIT);
}

int16_t adc_read_battery(void){
    return adc_read_channel_mv(ADC_BAT_CHANNEL);
}

static int16_t adc_read_channel_raw(adc_channel_t channel){
    int raw = 0;

    esp_err_t err = adc_oneshot_read(adc_handle, channel, &raw);
    if (err != ESP_OK) {
        return -1;
    }

    return (int16_t)raw;
}

static int16_t adc_read_channel_mv(adc_channel_t channel){
    int raw = adc_read_channel_raw(channel);

    if (raw < 0) {
        return -1;
    }

    if (!adc_calibrated) {
        return raw;
    }

    int voltage_mv = 0;
    esp_err_t err = adc_cali_raw_to_voltage(adc_cali_handle, raw, &voltage_mv);
    if (err != ESP_OK) {
        return -1;
    }

    return (int16_t)voltage_mv;
}

static bool adc_calibration_init(void){
    adc_cali_line_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    esp_err_t err = adc_cali_create_scheme_line_fitting(&cali_config, &adc_cali_handle);

    if (err == ESP_OK) {
        return true;
    }

    return false;
}