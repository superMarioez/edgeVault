#pragma once
#include "driver/spi_master.h"
#include "driver/sdspi_host.h"
#include "esp_vfs_fat.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

namespace sdlogger {

    class SDLogger {

        public:
            explicit SDLogger(spi_host_device_t spi_bus, const char* mount_path);
            ~SDLogger();

            SDLogger(const SDLogger&) = delete;
            SDLogger& operator=(const SDLogger&) = delete;
            SDLogger(SDLogger&& other) noexcept;
            SDLogger& operator=(SDLogger&& other);

        private:

            char mnt_path_[16] = {}; // owns the VFS routing path
            sdmmc_card_t* card_h_ = nullptr; // owns the SD card protocol state
    };

}