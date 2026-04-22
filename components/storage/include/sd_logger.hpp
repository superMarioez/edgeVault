#pragma once
#include "driver/spi_master.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
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
            SDLogger& operator=(SDLogger&& other);

        private:

            char mnt_path_[16] = {}; // owns the VFS routing path
            sdspi_dev_handle_t dev_h_ = {}; // owns the physical SPI device lock
            sdmmc_card_t* card_h_ = nullptr; // owns the SD card protocol state
    };

}