#include "adc.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_adc/adc_oneshot.h"

#define ADC_SAMPLES_PER_CHANNEL 16

static const char* TAG = "ADC";

static QueueHandle_t adc_queue;
static adc_oneshot_unit_handle_t adc_handle;

static const adc_channel_t adc_channels[ADC_CHANNEL_COUNT] = {
    ADC_CHANNEL_0, // GPIO36 / VP
    ADC_CHANNEL_3, // GPIO39 / VN
    ADC_CHANNEL_4, // GPIO32
    ADC_CHANNEL_5, // GPIO33
    ADC_CHANNEL_6, // GPIO34
    ADC_CHANNEL_7  // GPIO35
};

static void adc_task(void* args){
    uint32_t seq = 0;

    while (1) {
        adc_sample_t sample = {0};

        sample.seq = seq++;
        sample.timestamp_ms = esp_timer_get_time() / 1000;

        for (int ch = 0; ch < ADC_CHANNEL_COUNT; ch++) {
            int sum = 0;
            int valid_samples = 0;

            for (int i = 0; i < ADC_SAMPLES_PER_CHANNEL; i++) {
                int raw = 0;

                esp_err_t err = adc_oneshot_read(adc_handle, adc_channels[ch], &raw);
                if (err == ESP_OK) {
                    sum += raw;
                    valid_samples++;
                }
            }

            sample.raw[ch] = valid_samples > 0 ? sum / valid_samples : -1;
        }

        xQueueOverwrite(adc_queue, &sample);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void adc_init(void){
    adc_queue = xQueueCreate(1, sizeof(adc_sample_t));
    if (adc_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create ADC queue");
        return;
    }

    adc_oneshot_unit_init_cfg_t unit_cfg = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_cfg, &adc_handle));

    adc_oneshot_chan_cfg_t chan_cfg = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    for (int i = 0; i < ADC_CHANNEL_COUNT; i++) {
        ESP_ERROR_CHECK(adc_oneshot_config_channel(
            adc_handle,
            adc_channels[i],
            &chan_cfg
        ));
    }
}

void adc_start(void){
    xTaskCreate(adc_task, "adc_task", 
        4096, NULL, 5, NULL);
}

bool adc_get_latest(adc_sample_t *sample){
    if (sample == NULL || adc_queue == NULL) {
        return false;
    }

    return xQueueReceive(adc_queue, sample, 0) == pdTRUE;
}