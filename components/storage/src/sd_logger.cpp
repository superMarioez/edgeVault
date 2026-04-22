#include "sd_logger.hpp"


namespace sdlogger {


    SDLogger::SDLogger(spi_host_device_t spi_bus) {

        sdspi_device_config_t dev_cfg = {};
        dev_cfg.gpio_cs = static_cast<gpio_num_t>(CONFIG_EV_SDSPI_CS);
        dev_cfg.host_id = spi_bus;

        ESP_ERROR_CHECK(sdspi_host_init_device(&dev_cfg, &dev_h_));

        // internal hw controller (SPI) configurations
        sdmmc_host_t host_cfg_input = SDSPI_HOST_DEFAULT();

        // sdspi device configurations
        sdspi_device_config_t dev_cfg = SDSPI_DEVICE_CONFIG_DEFAULT();

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

        ESP_LOGI();

    }


}