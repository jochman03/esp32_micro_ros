#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

#include "sensors.h"

#define ADC_SAMPLES_PER_CHANNEL 16

static const char* TAG = "SENSORS";

static QueueHandle_t sensors_queue;
static adc_oneshot_unit_handle_t adc_handle;

static const adc_channel_t adc_channels[ADC_CHANNEL_COUNT] = {
    ADC_CHANNEL_0, ADC_CHANNEL_3, ADC_CHANNEL_6, ADC_CHANNEL_4, ADC_CHANNEL_5,

};

static const gpio_num_t digital_sensor_gpios[DIGITAL_CHANNEL_COUNT] = {
    GPIO_NUM_25, GPIO_NUM_26, GPIO_NUM_27, GPIO_NUM_14, GPIO_NUM_13};

static void sensors_task(void* args) {
    uint32_t seq = 0;

    while (1) {
        sensors_sample_t sample = {0};

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
        for (int i = 0; i < DIGITAL_CHANNEL_COUNT; i++) {
            int level = gpio_get_level(digital_sensor_gpios[i]);

            sample.raw[ADC_CHANNEL_COUNT + i] = level ? 4095 : 0;
        }

        xQueueOverwrite(sensors_queue, &sample);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void sensors_init(void) {
    uint64_t pin_mask = 0;

    for (int i = 0; i < DIGITAL_CHANNEL_COUNT; i++) {
        pin_mask |= 1ULL << digital_sensor_gpios[i];
    }

    gpio_config_t input_cfg = {
        .pin_bit_mask = pin_mask,
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&input_cfg));

    sensors_queue = xQueueCreate(1, sizeof(sensors_sample_t));
    if (sensors_queue == NULL) {
        ESP_LOGE(TAG, "Failed to create sensors_queue");
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
        ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, adc_channels[i], &chan_cfg));
    }
}

void sensors_start(void) {
    xTaskCreate(sensors_task, "adc_task", 4096, NULL, 5, NULL);
}

bool sensors_get_latest(sensors_sample_t* sample) {
    if (sample == NULL || sensors_queue == NULL) {
        return false;
    }

    return xQueueReceive(sensors_queue, sample, 0) == pdTRUE;
}