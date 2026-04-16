#include "data_pipeline.hpp"


namespace datapipeline {

    static const char *TAG = "data pipeline";

    void data_pipeline_task(void *pvParameters) {

        ESP_LOGI(TAG, "start");
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

}