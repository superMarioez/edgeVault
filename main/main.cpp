#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ev_types.hpp"
#include "sensor_hub.hpp"
#include "data_pipeline.hpp"
#include "network_manager.hpp"
#include "config_manager.hpp"

namespace {
const char* TAG = "edgevault";
sensorhub::SensorHubContext sensor_hub_params = {};
datapipeline::DataPipelineContext data_pipeline_params = {};
}

extern "C" void app_main(void)
{

    ESP_LOGI(TAG, "EdgeVault v0.1.0");
    
    /* initialize the flash nvs partition */
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // create the system event group
    EventGroupHandle_t system_events_handle = xEventGroupCreate();
    ESP_ERROR_CHECK(system_events_handle ? ESP_OK : ESP_ERR_NO_MEM);

    // initializing the i2c master bus
    i2c_master_bus_config_t i2c_bus_cfg = {};
    i2c_bus_cfg.i2c_port = I2C_NUM_0;
    i2c_bus_cfg.sda_io_num = static_cast<gpio_num_t>(CONFIG_EV_I2C_SDA_GPIO);
    i2c_bus_cfg.scl_io_num = static_cast<gpio_num_t>(CONFIG_EV_I2C_SCL_GPIO);
    i2c_bus_cfg.flags.enable_internal_pullup = true;
    i2c_bus_cfg.flags.allow_pd = false;
    i2c_bus_cfg.glitch_ignore_cnt = 7;
    i2c_bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;

    // provide the system events handle tasks that consume it.
    sensor_hub_params.system_events_h_ = system_events_handle;
    data_pipeline_params.system_events_h_ = system_events_handle;

    // create the i2c master bus
    i2c_master_bus_handle_t i2c_bus = {};
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &i2c_bus));

    // provide the master bus handle to the local sensor task context
    sensor_hub_params.i2c_bus_h_ = i2c_bus;

    // create the sensor data queue
    QueueHandle_t sensor_q_h = xQueueCreate(ev::config::SENSOR_QUEUE_DEPTH, sizeof(ev::SensorReading));
    ESP_ERROR_CHECK(sensor_q_h ? ESP_OK : ESP_ERR_NO_MEM);

    // provide the queue handle to it's producers/consumers
    sensor_hub_params.data_queue_h_ = sensor_q_h;
    data_pipeline_params.data_queue_h_ = sensor_q_h;

    /* spawn the system's tasks */
    configASSERT(xTaskCreatePinnedToCore(
        sensorhub::local_sensor_task,
        "sensor_hub",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        static_cast<void*>(&sensor_hub_params),
        5,
        nullptr,
        1
    ) == pdPASS);

    configASSERT(xTaskCreatePinnedToCore(
        datapipeline::data_pipeline_task,
        "data_pipe",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        static_cast<void*>(&data_pipeline_params),
        4,
        nullptr,
        1
    ) == pdPASS);

    configASSERT(xTaskCreatePinnedToCore(
        networkmanager::network_manager_task,
        "net_mgr",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        static_cast<void*>(system_events_handle),
        6,
        nullptr,
        0
    ) == pdPASS);

}
