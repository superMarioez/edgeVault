#include "sensor_hub.hpp"
#include "lm75a.hpp"
#include "ds3231.hpp"
#include "adc_battery.hpp"
#include <cmath>
#include "driver/gpio.h"

namespace sensorhub {

    static const char *TAG = "local sensor task";

    static void send_or_discard_oldest(QueueHandle_t q_h, const ev::SensorReading& ssr_reading) {
        BaseType_t q_ret = xQueueSend(q_h, &ssr_reading, 0);
        if (q_ret == errQUEUE_FULL) {
            ev::SensorReading throwaway_sns_rdg = {};
            xQueueReceive(q_h, &throwaway_sns_rdg, 0);
            q_ret = xQueueSend(q_h, &ssr_reading, 0);
            ESP_LOGW(TAG, "Queue was full, discarded oldest value");
        }
    }

    void local_sensor_task(void *pvParameters) {

        ESP_LOGI(TAG, "sensor task start");

        // configure a status LED
        gpio_config_t gpio_cfg = {};
        gpio_cfg.mode = GPIO_MODE_OUTPUT;
        gpio_cfg.pull_up_en = GPIO_PULLUP_DISABLE;
        gpio_cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        gpio_cfg.pin_bit_mask = 1ULL << CONFIG_EV_STATUS_LED_GPIO;
        gpio_cfg.intr_type = GPIO_INTR_DISABLE;
        ESP_ERROR_CHECK(gpio_config(
            &gpio_cfg
        ));

        SensorHubContext* sensorHub_ctx = static_cast<SensorHubContext*>(pvParameters);
        ev::SensorReading sensor_readings = {};

        Lm75a temp_sensor(sensorHub_ctx->i2c_bus_h_);
        Ds3231 rtc_sensor(sensorHub_ctx->i2c_bus_h_);
        AdcBattery adc_unit;

        float temperature = 0.0f;
        tm out_time = {};
        int voltage = 0;

        esp_err_t ret = ESP_FAIL;

        for (;;) {

            // a status LED indicating the system is actively sampling
            gpio_set_level(
                static_cast<gpio_num_t>(CONFIG_EV_STATUS_LED_GPIO),
                1
            );

            /* timestamp calculation */
            ret = rtc_sensor.read_time_date(out_time);
            if (ret != ESP_OK) {
                ESP_LOGW(TAG, "DS3231 read failed: %s", esp_err_to_name(ret));
                sensor_readings.timestamp_ms_ = 0;
            }
            else {
                time_t ss = mktime(&out_time);
                if (ss != static_cast<time_t>(-1)) sensor_readings.timestamp_ms_ = static_cast<int64_t>(ss) * 1000;
                else sensor_readings.timestamp_ms_ = 0;
            }

            /* voltage read */
            sensor_readings.sensor_ = ev::SensorSource::Adc;
            sensor_readings.register_addr_ = 0;
            ret = adc_unit.read_voltage_mv(voltage);
            if (ret != ESP_OK) {
                sensor_readings.quality_ = ev::Quality::Error;
                sensor_readings.value_ = NAN;
                xEventGroupSetBits(
                    sensorHub_ctx->system_events_h_,
                    ev::to_bits(ev::SystemEvent::SensorError)
                );
                ESP_LOGW(
                    TAG,
                    "ADC read failed: %s",
                    esp_err_to_name(ret)
                );
            }
            else {
                sensor_readings.quality_ = ev::Quality::Good;
                sensor_readings.value_ = static_cast<float>(voltage);
            }

            send_or_discard_oldest(sensorHub_ctx->data_queue_h_, sensor_readings);

            /* temperature read */
            sensor_readings.sensor_ = ev::SensorSource::Lm75a;
            sensor_readings.register_addr_ = 0;

            ret = temp_sensor.read_temperature(temperature);
            if (ret != ESP_OK) {
                ESP_LOGW(TAG, "LM75A read failed: %s", esp_err_to_name(ret));
                sensor_readings.quality_ = ev::Quality::Error;
                xEventGroupSetBits(
                    sensorHub_ctx->system_events_h_,
                    ev::to_bits(ev::SystemEvent::SensorError)
                );
                sensor_readings.value_ = NAN;
            }
            else {
                sensor_readings.quality_ = ev::Quality::Good;
                sensor_readings.value_ = temperature;
            }
            
            send_or_discard_oldest(sensorHub_ctx->data_queue_h_, sensor_readings);

            gpio_set_level(
                static_cast<gpio_num_t>(CONFIG_EV_STATUS_LED_GPIO),
                0
            );
            vTaskDelay(pdMS_TO_TICKS(CONFIG_EV_SENSOR_POLL_INTERVAL_MS));
        }
    }
}