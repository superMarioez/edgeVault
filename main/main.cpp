#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ev_types.hpp"
#include "sensor_hub.hpp"
#include "data_pipeline.hpp"
#include "network_manager.hpp"
#include "lm75a.hpp"
#include "ds3231.hpp"

namespace {
const char* TAG = "edgevault";
sensorhub::SensorTaskContext sensor_hub_data = {};
}

extern "C" void app_main(void)
{

    ESP_LOGI(TAG, "EdgeVault v0.1.0");

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

    sensor_hub_data.system_events_handle_ = system_events_handle;

    // create the i2c master bus
    ESP_ERROR_CHECK(i2c_new_master_bus(&i2c_bus_cfg, &sensor_hub_data.i2c_handle_));

    configASSERT(xTaskCreatePinnedToCore(
        sensorhub::local_sensor_task,
        "sensor_hub",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        static_cast<void*>(&sensor_hub_data),
        5,
        nullptr,
        1
    ) == pdPASS);

    configASSERT(xTaskCreatePinnedToCore(
        datapipeline::data_pipeline_task,
        "data_pipe",
        ev::config::TASK_STACK_SIZE_DEFAULT,
        static_cast<void*>(system_events_handle),
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
