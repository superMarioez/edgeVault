#include "data_pipeline.hpp"
#include "config_manager.hpp"
#include "sd_logger.hpp"
#include "inttypes.h"
#include <cmath>
#include <array>
#include "stdio.h"

namespace datapipeline {

    static const char *TAG = "data pipeline";


    static constexpr std::array<const char*, static_cast<size_t>(ev::SensorSource::Count)> nvs_keys = 
    {
        "cal_modbus",
        "cal_lm75a",
        "cal_ds3231",
        "cal_adc",
        "cal_bme280"
    };
    static_assert(nvs_keys.size() == static_cast<size_t>(ev::SensorSource::Count),
    "nvs_keys must match SensorSource count");

    void data_pipeline_task(void *pvParameters) {

        ESP_LOGI(TAG, "start");

        DataPipelineContext* dp_ctx = static_cast<DataPipelineContext*>(pvParameters);
        ev::SensorReading ssr_buff;
        storage::ConfigManager cfg_mgr("ev_cal");

        std::array<float, static_cast<size_t>(ev::SensorSource::Count)> offsets = {};
        for (size_t i = 0; i < static_cast<size_t>(ev::SensorSource::Count); ++i) {
            esp_err_t cfg_mgr_ret = cfg_mgr.get_float(nvs_keys[i], offsets[i]);
            if (cfg_mgr_ret != ESP_OK && cfg_mgr_ret != ESP_ERR_NVS_NOT_FOUND) {
                ESP_LOGW(TAG, "Failed to retrieve calibration value from %s: %s",
                    ev::ssr_id_to_str(static_cast<ev::SensorSource>(i)).sensor,
                    esp_err_to_name(cfg_mgr_ret));
            }
        }

        // create an sd logger
        sdlogger::SDLogger logger(dp_ctx->spi_host_, "/sdcard");

        // open a csv file
        FILE* log_file = fopen("/sdcard/log.csv", "a+");

        if (log_file != nullptr) {
            if (!ftell(log_file)) {
                fprintf(log_file, "timestamp_ms,sensor,value,quality,register_addr\n");
            }
        }

        for (;;) {

            BaseType_t ret = xQueueReceive(
                dp_ctx->data_queue_h_,
                &ssr_buff,
                pdMS_TO_TICKS(5000)
            );

            if (ret != pdTRUE) {

                ESP_LOGW(
                    TAG,
                    "Unable to receive data within the specified timeout"
                );

                continue;
            }

            if (ssr_buff.quality_ == ev::Quality::Error) {
                ESP_LOGW(
                    TAG,
                    "[%" PRId64 "] Failure reading %s",
                    ssr_buff.timestamp_ms_
                    ,ev::ssr_id_to_str(ssr_buff.sensor_).sensor
                );
            }

            else {

                if (std::isnan(ssr_buff.value_))
                    ESP_LOGE(
                        TAG,
                        "[%" PRId64 "] %s | NAN",
                        ssr_buff.timestamp_ms_,
                        ev::ssr_id_to_str(ssr_buff.sensor_).sensor
                    );
                    
                else {
                    ssr_buff.value_ += offsets[static_cast<uint8_t>(ssr_buff.sensor_)];
                    ESP_LOGI(
                        TAG,
                        "[%" PRId64 "] %s | value = %0.2f %s | quality= %s ",
                        ssr_buff.timestamp_ms_,
                        ev::ssr_id_to_str(ssr_buff.sensor_).sensor,
                        ssr_buff.value_,
                        ev::ssr_id_to_str(ssr_buff.sensor_).unit,
                        ev::quality_to_str(ssr_buff.quality_)
                    );
                }
            }
        }
    }

}