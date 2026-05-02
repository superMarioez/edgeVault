/* public: task entry + context struct */
#pragma once
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/gpio.h"
#include "driver/uart.h"

namespace modbus_master {


    struct ModbusMasterContext {
        
        QueueHandle_t q_h_;
        EventGroupHandle_t sys_events_h_;
        uart_port_t uart_port_;
        gpio_num_t de_re_pin_;

    };

    void modbus_master_task(void* pvParameters);

}