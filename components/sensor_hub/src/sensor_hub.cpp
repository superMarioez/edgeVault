#include "sensor_hub.hpp"

const char *TAG = "local sensor task";

namespace sensorHub {

    void local_sensor_task(void *pvParamters) {

        ESP_LOGI(TAG, "lst start");
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}