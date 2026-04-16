#include "sensor_hub.hpp"
#include "data_pipeline.hpp"
#include "wifi_manager.hpp"

static const char* TAG = "edgevault";

extern "C" void app_main(void)
{

    ESP_LOGI(TAG, "EdgeVault v0.1.0");

    ESP_ERROR_CHECK(xTaskCreatePinnedToCore(
        sensorHub::local_sensor_task,
        "Local Sensor Task",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        nullptr,
        5,
        nullptr,
        1
    ));
    ESP_ERROR_CHECK(xTaskCreatePinnedToCore(
        dataPipeline::data_pipeline_task,
        "Data Pipeline Task",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        nullptr,
        4,
        nullptr,
        1
    ));
    ESP_ERROR_CHECK(xTaskCreatePinnedToCore(
        wifiManager::network_manager_task,
        "Network Manager Task",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        nullptr,
        6,
        nullptr,
        0
    ));

}
