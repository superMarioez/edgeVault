#pragma once

#include "ev_types.hpp"
#include "sd_logger.hpp"

namespace datapipeline {

    void data_pipeline_task(void *pvParameters);

    struct DataPipelineContext {
        EventGroupHandle_t system_events_h_;
        QueueHandle_t data_queue_h_;
        spi_host_device_t spi_host_;
    };
}