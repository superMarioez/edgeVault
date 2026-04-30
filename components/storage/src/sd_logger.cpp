#include "sd_logger.hpp"
#include "string.h"

namespace sdlogger {

    static const char* TAG = "SDCard Logger";

    SDLogger::SDLogger(spi_host_device_t spi_bus, const char* mount_path) {

        // set the mount path
        strncpy(mnt_path_, mount_path, sizeof(mnt_path_));

        // sdspi device configurations
        sdspi_device_config_t dev_cfg = {
            .host_id = spi_bus,
            .gpio_cs = static_cast<gpio_num_t>(CONFIG_EV_SDSPI_CS),
            .gpio_cd = SDSPI_SLOT_NO_CD,
            .gpio_wp = SDSPI_SLOT_NO_WP,
            .gpio_int = GPIO_NUM_NC,
            .gpio_wp_polarity = SDSPI_IO_ACTIVE_LOW,
            .duty_cycle_pos = 0
        };

        // internal hw controller (SPI) configurations
        sdmmc_host_t host_cfg_input = SDSPI_HOST_DEFAULT();

        // filesystem rules configurations
        esp_vfs_fat_mount_config_t mnt_cfg = {
            .format_if_mount_failed = false,
            .max_files = 1,
            .allocation_unit_size = 0,
            .disk_status_check_enable = false,
            .use_one_fat = true
        };

        /*
        - set the path of the SD card into the OS's master directory
        - set the configurations of the esp internal hardware, to define it's behaviour
        - set the configurations of the sdspi device
        - set the filesystem rules
        - get the handle of the sd card initialized
        */
        ESP_ERROR_CHECK(esp_vfs_fat_sdspi_mount(
            static_cast<const char*>(mnt_path_),
            &host_cfg_input,
            &dev_cfg,
            &mnt_cfg,
            &card_h_
        ));

        ESP_LOGI(
            TAG,
            "sd mounted successfully"
        );

        // log the card info
        ESP_LOGI(
            TAG,
            "card name: %s | type: %s | capacity (MBs): %d | sec size: %d",
            card_h_->cid.name,
            (card_h_->is_mem)? "memory card" : (card_h_->is_mmc)? "mmc card" : "io card",
            (card_h_->csd.capacity * card_h_->csd.sector_size) / (1024 * 1024),
            card_h_->csd.sector_size
        );

    }

    SDLogger::~SDLogger() {
        if (card_h_ != nullptr) {
            esp_err_t ret = esp_vfs_fat_sdcard_unmount(mnt_path_, card_h_);
            if (ret != ESP_OK) ESP_LOGE(TAG, "Unmounting unsuccessful: %s", esp_err_to_name(ret));
            else card_h_ = nullptr;
        }
    }

    SDLogger::SDLogger(SDLogger&& other) noexcept : card_h_(other.card_h_)
    {
        strncpy(mnt_path_, other.mnt_path_, sizeof(mnt_path_));

        other.card_h_ = nullptr;
        other.mnt_path_[0] = '\0';
    }

    SDLogger& SDLogger::operator=(SDLogger&& other) noexcept {

        // check for self assignment
        if (this == &other) return *this;

        if (card_h_ != nullptr) esp_vfs_fat_sdcard_unmount(mnt_path_, card_h_);
        mnt_path_[0] = '\0';

        card_h_ = other.card_h_;
        strncpy(mnt_path_, other.mnt_path_, sizeof(mnt_path_));

        other.card_h_ = nullptr;
        other.mnt_path_[0] = '\0';

        return *this;
    }

}