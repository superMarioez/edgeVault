#pragma once

#include "nvs_flash.h"
#include "driver/nvs.h"
#include "esp_err.h"
#include <cstdint>

namespace storage {

    class ConfigManager {


        public:
            
            explicit ConfigManager(const char* ns);
            ~ConfigManager();

            ConfigManager(const ConfigManager&) = delete;
            ConfigManager& operator=(const ConfigManager&) = delete;
            ConfigManager(ConfigManager&& other) noexcept;
            ConfigManager& operator=(ConfigManager&& other) noexcept;

            esp_err_t get_i32(const char* key, int32_t& out) const;
            esp_err_t set_i32(const char* key, int32_t value);

            esp_err_t get_float(const char* key, float& out) const;
            esp_Err_t set_float(const char* key, float value);

            esp_err_t get_str(const char* key, char* out, size_t max_len) const;
            esp_err_t set_str(const char* key, const char* value);

            esp_err_t commit();

        private:
            nvs_handle_t handle_;            


    };


}