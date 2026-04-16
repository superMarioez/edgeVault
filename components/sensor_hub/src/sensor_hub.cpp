#include "sensor_hub.hpp"

namespace sensorhub {

    static const char *TAG = "local sensor task";

    void local_sensor_task(void *pvParameters) {

        ESP_LOGI(TAG, "lst start");
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}