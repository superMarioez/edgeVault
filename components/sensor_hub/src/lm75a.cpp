#include "lm75a.hpp"



namespace sensorhub {

    static constexpr uint32_t I2C_XFER_TIMEOUT_MS = 100;

    Lm75a::Lm75a(i2c_master_bus_handle_t bus) {
        
        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = CONFIG_EV_LM75A_I2C_ADDR;
        dev_cfg.scl_speed_hz = CONFIG_EV_I2C_FREQ_HZ;

        ESP_ERROR_CHECK(i2c_master_bus_add_device(
            bus,
            &dev_cfg,
            &dev_handle_
        ));
    }

    Lm75a::~Lm75a() {
        if (dev_handle_ != nullptr) i2c_master_bus_rm_device(dev_handle_);
    }

    Lm75a::Lm75a(Lm75a&& other) noexcept : dev_handle_(other.dev_handle_) {

        other.dev_handle_ = nullptr;

    }

    Lm75a& Lm75a::operator=(Lm75a&& other) noexcept {

        if (this == &other) return *this;

        if (dev_handle_ != nullptr) i2c_master_bus_rm_device(dev_handle_);

        dev_handle_ = other.dev_handle_;
        other.dev_handle_ = nullptr;

        return *this;
    }

    esp_err_t Lm75a::read_temperature(float& out_temp) const {

        uint8_t buff[2] = {};

        esp_err_t ret = i2c_master_transmit_receive(
            dev_handle_,
            &TEMP_REG,
            1,
            buff,
            2,
            I2C_XFER_TIMEOUT_MS
        );

        if (ret != ESP_OK) return ret;

        uint16_t raw_temp = (buff[0] << 8) | buff[1];
        raw_temp >>= 5;

        // check bit 10 for negative temperature
        if (0x0400 & raw_temp) raw_temp |= 0xF800;

        out_temp = static_cast<int16_t>(raw_temp) * 0.125f;
        
        return ret;
    }

}