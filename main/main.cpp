#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ev_types.hpp"
#include "sensor_hub.hpp"
#include "data_pipeline.hpp"
#include "network_manager.hpp"

static const char* TAG = "edgevault";

extern "C" void app_main(void)
{

    ESP_LOGI(TAG, "EdgeVault v0.1.0");

    EventGroupHandle_t system_events_handle = xEventGroupCreate();
    ESP_ERROR_CHECK(system_events_handle ? ESP_OK : ESP_ERR_NO_MEM);

    configASSERT(xTaskCreatePinnedToCore(
        sensorhub::local_sensor_task,
        "sensor_hub",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        (void*)system_events_handle,
        5,
        nullptr,
        1
    ) == pdPASS);

    configASSERT(xTaskCreatePinnedToCore(
        datapipeline::data_pipeline_task,
        "data_pipe",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        (void*)system_events_handle,
        4,
        nullptr,
        1
    ) == pdPASS);

    configASSERT(xTaskCreatePinnedToCore(
        networkmanager::network_manager_task,
        "net_mgr",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        (void*)system_events_handle,
        6,
        nullptr,
        0
    ) == pdPASS);

}
