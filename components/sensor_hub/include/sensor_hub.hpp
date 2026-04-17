#pragma once

#include "ev_types.hpp"
#include "driver/i2c_master.h"

namespace sensorhub {

    void local_sensor_task(void *pvParameters);

    struct SensorTaskContext {
        i2c_master_bus_handle_t i2c_handle_;
        EventGroupHandle_t system_events_handle_;
    };

}