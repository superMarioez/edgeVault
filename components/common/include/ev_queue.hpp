#pragma once
#include "freertos/FreeRTOS.h"
#include "esp_log.h"

namespace ev {

    void queue_send_drop_oldest(const char* tag, QueueHandle_t q_h, const ev::SensorReading& ssr_reading) {
        BaseType_t q_ret = xQueueSend(q_h, &ssr_reading, 0);
        if (q_ret == errQUEUE_FULL) {
            ev::SensorReading throwaway_sns_rdg = {};
            xQueueReceive(q_h, &throwaway_sns_rdg, 0);
            q_ret = xQueueSend(q_h, &ssr_reading, 0);
            ESP_LOGW(tag, "Queue was full, discarded oldest value");
        }
    }

}