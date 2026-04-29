#include "data_pipeline.hpp"
#include "config_manager.hpp"
#include "sd_logger.hpp"
#include "inttypes.h"
#include <cmath>
#include <array>
#include "stdio.h"

namespace datapipeline {

    static const char *TAG = "data pipeline";

    constexpr const char* MOUNT_PATH = "/sdcard";

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

    static void file_log_row(FILE* file_h, DataPipelineContext* dp_context, ev::SensorReading ssr_rdgs) {
        int fret = fprintf(
            file_h,
            "%" PRId64 ",%s,%0.2f,%s,%s,%d\n",
            ssr_rdgs.timestamp_ms_,
            ev::ssr_id_to_str(ssr_rdgs.sensor_).sensor,
            ssr_rdgs.value_,
            ev::ssr_id_to_str(ssr_rdgs.sensor_).unit,
            ev::quality_to_str(ssr_rdgs.quality_),
            ssr_rdgs.register_addr_);

        if (fret < 0) {
            ESP_LOGE(TAG, "data row logging failed");
            xEventGroupSetBits(
                dp_context->system_events_h_,
                ev::to_bits(ev::SystemEvent::StorageError)
            );
        }
        else {
            fret = fflush(file_h);
            if (fret < 0) {
                xEventGroupSetBits(
                    dp_context->system_events_h_,
                    ev::to_bits(ev::SystemEvent::StorageError)
                );
                ESP_LOGE(TAG, "row logging failed");
            }
        }
        
    }

    void data_pipeline_task(void *pvParameters) {

        bool terminate = false;

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

    {

        // create an sd logger
        sdlogger::SDLogger logger(dp_ctx->spi_host_, MOUNT_PATH);

        char file_path[64];
        snprintf(file_path, sizeof(file_path), "%s/log.csv", MOUNT_PATH);

        // open a csv file
        FILE* log_file = fopen(file_path, "a+");

        int fret = -1;

        // handle fopen() failure
        if (log_file == nullptr) {
            xEventGroupSetBits(dp_ctx->system_events_h_, ev::to_bits(ev::SystemEvent::StorageError));
            terminate = true;
            ESP_LOGE(TAG, "couldn't open log file");
        }

        else {
            // placing the csv header
            fret = fseek(log_file, 0, SEEK_END);
            if (fret < 0) ESP_LOGE(TAG, "file cursor cannot be moved to the EOF");

            long size = ftell(log_file);
            if (size == 0) {
                ESP_LOGI(TAG, "new CSV file created, printing header");

                fret = fprintf(log_file, "timestamp_ms,sensor,value,unit,quality,register_addr\n");
                if (fret < 0) {
                    xEventGroupSetBits(dp_ctx->system_events_h_, ev::to_bits(ev::SystemEvent::StorageError));
                    ESP_LOGE(TAG, "file header cannot be printed");
                }
                else {
                    fret = fflush(log_file);
                    if (fret < 0) {
                        xEventGroupSetBits(dp_ctx->system_events_h_, ev::to_bits(ev::SystemEvent::StorageError));
                        ESP_LOGE(TAG, "header logging failed");
                    }
                }
            }
        }

        for (;;) {

            if (terminate) break;

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

                if (log_file != nullptr) {
                    file_log_row(log_file, dp_ctx, ssr_buff);
                }
            }

            else {

                if (std::isnan(ssr_buff.value_)) {
                    ESP_LOGE(
                        TAG,
                        "[%" PRId64 "] %s | NAN",
                        ssr_buff.timestamp_ms_,
                        ev::ssr_id_to_str(ssr_buff.sensor_).sensor
                    );

                if (log_file != nullptr) {
                    file_log_row(log_file, dp_ctx, ssr_buff);
                    }
                }
            
                else {
                    ssr_buff.value_ += offsets[static_cast<uint8_t>(ssr_buff.sensor_)];
                    
                    if (log_file != nullptr) {
                        file_log_row(log_file, dp_ctx, ssr_buff);
                    }

                    ESP_LOGI(
                        TAG,
                        "%" PRId64 " %s | value = %0.2f %s | quality= %s ",
                        ssr_buff.timestamp_ms_,
                        ev::ssr_id_to_str(ssr_buff.sensor_).sensor,
                        ssr_buff.value_,
                        ev::ssr_id_to_str(ssr_buff.sensor_).unit,
                        ev::quality_to_str(ssr_buff.quality_)
                    );
                }
            }
        }

        if (log_file != nullptr) {
        fret = fclose(log_file);
        if (fret < 0) ESP_LOGE(TAG, "log file couldn't be closed");
        }
    }

    vTaskDelete(nullptr);
}


    
}