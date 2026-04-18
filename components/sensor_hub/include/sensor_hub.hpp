#pragma once

#include "ev_types.hpp"
#include "driver/i2c_master.h"
#include <ctime>

namespace sensorhub {

    void local_sensor_task(void *pvParameters);

    struct SensorHubContext {
        i2c_master_bus_handle_t i2c_bus_h_;
        EventGroupHandle_t system_events_h_;
        QueueHandle_t data_queue_h_;
    };

}