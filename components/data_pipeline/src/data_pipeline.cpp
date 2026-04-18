#include "data_pipeline.hpp"
#include "inttypes.h"

namespace datapipeline {

    static const char *TAG = "data pipeline";

    void data_pipeline_task(void *pvParameters) {

        ESP_LOGI(TAG, "start");

        DataPipelineContext* dp_ctx = static_cast<DataPipelineContext*>(pvParameters);
        ev::SensorReading ssr_buff;

        for (;;) {

            BaseType_t ret = xQueueReceive(
                dp_ctx->data_queue_h_,
                &ssr_buff,
                pdMS_TO_TICKS(5000)
            );

            if (ret != pdTRUE) {

                ESP_LOGW(
                    TAG,
                    "Unable to receive data within the specified timeout",
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