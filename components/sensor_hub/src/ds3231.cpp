#include "ds3231.hpp"


namespace sensorhub {

    static constexpr uint32_t I2C_XFER_TIMEOUT_MS = 100;
    
    Ds3231::Ds3231(i2c_master_bus_handle_t bus) {

        i2c_device_config_t dev_cfg = {};
        dev_cfg.dev_addr_length = I2C_ADDR_BIT_LEN_7;
        dev_cfg.device_address = CONFIG_EV_DS3231_I2C_ADDR;
        dev_cfg.scl_speed_hz = CONFIG_EV_I2C_FREQ_HZ;
        
        ESP_ERROR_CHECK(i2c_master_bus_add_device(
            bus,
            &dev_cfg,
            &dev_handle_
        ));
    }

    Ds3231::~Ds3231() {
        if (dev_handle_ != nullptr) i2c_master_bus_rm_device(
            dev_handle_
        );
    }

    Ds3231::Ds3231(Ds3231&& other) noexcept : dev_handle_(other.dev_handle_) {
        other.dev_handle_ = nullptr;
    }

    Ds3231& Ds3231::operator=(Ds3231&& other) noexcept {
        if (this == &other) return *this;

        if (dev_handle_ != nullptr) i2c_master_bus_rm_device(dev_handle_);
        dev_handle_ = other.dev_handle_;
        other.dev_handle_ = nullptr;

        return *this;
    }

    esp_err_t Ds3231::read_time_date(struct tm& out_time) const {
        
        uint8_t buff[7] = {};

        esp_err_t ret = i2c_master_transmit_receive(
            dev_handle_,
            &TIME_REG,
            1,
            buff,
            NUM_TIME_REGS,
            I2C_XFER_TIMEOUT_MS
        );

        if (ret != ESP_OK) return ret;

        out_time.tm_sec = bcd_to_dec(buff[0]);
        out_time.tm_min = bcd_to_dec(buff[1]);
        out_time.tm_hour = bcd_to_dec(buff[2] & 0x3f);
        out_time.tm_mday = bcd_to_dec(buff[4]);
        out_time.tm_mon = bcd_to_dec(buff[5] & 0x1f) - 1;
        out_time.tm_year = bcd_to_dec(buff[6]) + 100;
        out_time.tm_isdst = -1;

        return ret;
    }

    uint8_t Ds3231::bcd_to_dec(uint8_t bcd) {
        uint8_t dec = 0;
        dec = (bcd >> 4) * 10 + (bcd & 0x0f);
        return dec;
    }

}