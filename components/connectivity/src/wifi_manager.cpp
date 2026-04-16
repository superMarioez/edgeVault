#include "wifi_manager.hpp"

namespace wifiManager {

    static const char *TAG = "wifi manager";

    void network_manager_task(void *pvParamters) {

        ESP_LOGI(TAG, "start");
        for (;;) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

}