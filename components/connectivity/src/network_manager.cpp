#include "network_manager.hpp"

namespace networkmanager {

    static const char *TAG = "network manager";

    void network_manager_task(void *pvParameters) {

        ESP_LOGI(TAG, "start");
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

}