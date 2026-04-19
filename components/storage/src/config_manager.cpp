#include "config_manager.hpp"
#include <cstring>

namespace storage {

    ConfigManager::ConfigManager(const char* ns) {

        ESP_ERROR_CHECK(nvs_open(
            ns,
            NVS_READWRITE,
            &handle_
        ));
    }

    ConfigManager::~ConfigManager() {
        if (handle_ != 0) nvs_close(handle_);
    }

    ConfigManager::ConfigManager(ConfigManager&& other) noexcept : handle_(other.handle_) {
        other.handle_ = 0;
    }

    ConfigManager& ConfigManager::operator=(ConfigManager&& other) noexcept {
        if (this == &other) return *this;

        nvs_close(handle_);
        handle_ = other.handle_;
        other.handle_ = 0;

        return *this;
    }

    esp_err_t ConfigManager::get_i32(const char* key, int32_t& out) const {
        return nvs_get_i32(
            handle_,
            key,
            &out
        );
    }

    esp_err_t ConfigManager::set_i32(const char* key, int32_t value) {
        return nvs_set_i32(
            handle_,
            key,
            value
        );
    }

    esp_err_t ConfigManager::get_float(const char* key, float& out) const {

        uint32_t out_u32 = 0; 
    
        esp_err_t ret = nvs_get_u32(
            handle_,
            key,
            &out_u32
        );

        if (ret == ESP_OK) memcpy(&out, &out_u32, sizeof(out));
        return ret;
    }

    esp_err_t ConfigManager::set_float(const char* key, float value) {
        
        uint32_t val_u32 = 0;
        memcpy(&val_u32, &value, sizeof(value));

        return nvs_set_u32(
            handle_,
            key,
            val_u32
        );
    }

    esp_err_t ConfigManager::get_str(const char* key, char* out, size_t* max_len) const {
        return nvs_get_str(
            handle_,
            key,
            out,
            max_len
        );
    }

    esp_err_t ConfigManager::set_str(const char* key, const char* value) {
        return nvs_set_str(
            handle_,
            key,
            value
        );
    }

    esp_err_t ConfigManager::commit() {
        return nvs_commit(handle_);
    }

}