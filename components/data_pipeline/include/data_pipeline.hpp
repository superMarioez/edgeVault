#pragma once

#include "ev_types.hpp"


namespace datapipeline {

    void data_pipeline_task(void *pvParameters);

    struct DataPipelineContext {
        EventGroupHandle_t system_events_h_;
        QueueHandle_t data_queue_h_;
    };
}