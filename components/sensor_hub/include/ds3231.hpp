#pragma once

#include <cstdint>
#include <ctime>
#include "esp_err.h"
#include "driver/i2c_master.h"


namespace sensorhub {

    class Ds3231 {

        public:

            explicit Ds3231(i2c_master_bus_handle_t bus);
            ~Ds3231();
            
            Ds3231(const Ds3231&) = delete;
            Ds3231& operator=(const Ds3231&) = delete;

            Ds3231(Ds3231&& other) noexcept;
            Ds3231& operator=(Ds3231&& other) noexcept;

            esp_err_t read_time_date(struct tm& out_time) const;

        private:

            i2c_master_dev_handle_t dev_handle_ = nullptr;

            static constexpr uint8_t TIME_REG = 0x00;
            static constexpr uint8_t NUM_TIME_REGS = 7;

            static uint8_t bcd_to_dec(uint8_t bcd);

    };

}