#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "ev_types.hpp"
#include "sensor_hub.hpp"
#include "data_pipeline.hpp"
#include "network_manager.hpp"
#include "config_manager.hpp"
#include "driver/spi_master.h"
#include "sd_logger.hpp"
#include "modbus_transport.hpp"
#include <utility>

namespace {
const char* TAG = "edgevault";
sensorhub::SensorHubContext sensor_hub_params = {};
datapipeline::DataPipelineContext data_pipeline_params = {};
}

extern "C" void app_main(void)
{

    ESP_LOGI(TAG, "EdgeVault v0.1.0");

    /* ModbusTransport Destructor test */
    {
        modbus_transport::ModbusTransport scoped_transport(UART_NUM_1, 17, 16, 18, 9600);
        ESP_LOGI(TAG, "Scoped transport object created successfully!");        
    }
    ESP_LOGI(TAG, "Exited artificial scope. Driver should be deleted by now!");
    
    /* Move Constructor test */
    {
        modbus_transport::ModbusTransport transport_A(UART_NUM_1, 17, 16, 18, 9600);
        ESP_LOGI(TAG, "transport_A created successfully!");
        modbus_transport::ModbusTransport(std::move(transport_A));
        ESP_LOGI(TAG, "transport_B should be owning transport_A resources by now, A is just an empty husk!");
    }
    ESP_LOGI(TAG, "Both objects should be destroyed by now!");
    
    /* Move Assignment Operator test */
    {
        modbus_transport::ModbusTransport transport_A(UART_NUM_1, 17, 16, 18, 9600);
        ESP_LOGI(TAG, "transport_A created successfully!");
        
        modbus_transport::ModbusTransport transport_B(UART_NUM_2, 4, 5, 6, 9600);
        ESP_LOGI(TAG, "transport_B created successfully!");

        transport_B = std::move(transport_A);
        ESP_LOGI(TAG, "transport_B should be owning transport_A resources by now");
    }
    ESP_LOGI(TAG, "Both objects should be destroyed by now!");

    
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

    // provide the system events handle tasks that consume it.
    sensor_hub_params.system_events_h_ = system_events_handle;
    data_pipeline_params.system_events_h_ = system_events_handle;

    /* initializing the i2c master bus */
    i2c_master_bus_config_t i2c_bus_cfg = {};
    i2c_bus_cfg.i2c_port = I2C_NUM_0;
    i2c_bus_cfg.sda_io_num = static_cast<gpio_num_t>(CONFIG_EV_I2C_SDA_GPIO);
    i2c_bus_cfg.scl_io_num = static_cast<gpio_num_t>(CONFIG_EV_I2C_SCL_GPIO);
    i2c_bus_cfg.flags.enable_internal_pullup = true;
    i2c_bus_cfg.flags.allow_pd = false;
    i2c_bus_cfg.glitch_ignore_cnt = 7;
    i2c_bus_cfg.clk_source = I2C_CLK_SRC_DEFAULT;

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

    /* SPI Bus Initialization */

    spi_bus_config_t spi_cfg = {};
    spi_cfg.mosi_io_num = CONFIG_EV_SPI_MOSI_PIN;
    spi_cfg.miso_io_num = CONFIG_EV_SPI_MISO_PIN;
    spi_cfg.sclk_io_num = CONFIG_EV_SPI_SCLK_PIN;
    spi_cfg.max_transfer_sz = CONFIG_EV_SPI_MAX_TSFR_SIZE;
    spi_cfg.quadwp_io_num = -1;
    spi_cfg.quadhd_io_num = -1;
    spi_cfg.isr_cpu_id = ESP_INTR_CPU_AFFINITY_1;

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &spi_cfg, SPI_DMA_CH_AUTO));

    data_pipeline_params.spi_host_ = SPI2_HOST;

    while (true) {

        vTaskDelay(pdMS_TO_TICKS(1000));
            
    }

    
    /* spawn the system's tasks */
    /*
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
    */
}
