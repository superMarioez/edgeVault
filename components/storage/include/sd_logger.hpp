#pragma once
#include "driver/spi_master.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace sdlogger {

    struct SDLoggerContext {
        spi_host_device_t spi_host_;
        EventGroupHandle_t system_events_h_;
    };


    class SDLogger {

        public:
            explicit SDLogger(spi_host_device_t spi_bus);
            ~SDLogger();

            SDLogger(const SDLogger&) = delete;
            SDLogger& operator=(const SDLogger&) = delete;
            SDLogger(SDLogger&& other) noexcept;
            SDLogger& operator=(SDLogger&& other) = delete;

        private:

            spi_device_handle_t dev_h_;

    };

}