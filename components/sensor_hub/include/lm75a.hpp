#pragma once

#include "driver/i2c_master.h"
#include "esp_err.h"
#include <cstdint>

namespace sensorhub {

    class Lm75a {

        public:
            explicit Lm75a(i2c_master_bus_handle_t bus);
            ~Lm75a();

            Lm75a(const Lm75a&) = delete;
            Lm75a& operator=(const Lm75a&) = delete;

            Lm75a(Lm75a&& other) noexcept;
            Lm75a& operator=(Lm75a&& other) noexcept;

            esp_err_t read_temperature(float& out_temp) const;

        private:

            i2c_master_dev_handle_t dev_handle_;
            static constexpr uint8_t TEMP_REG = 0x00;

    };

}