#pragma once
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

namespace sensorhub {


    class AdcBattery {

        public:

            explicit AdcBattery();
            ~AdcBattery();

            AdcBattery(const AdcBattery&) = delete;
            AdcBattery& operator=(const AdcBattery&) = delete;
            AdcBattery(AdcBattery&& other) noexcept;
            AdcBattery& operator=(AdcBattery&& other) noexcept;

            esp_err_t read_voltage_mv(uint32_t& out_mv) const;

        private:

            adc_oneshot_unit_handle_t adc_handle_ = {};
            adc_cali_handle_t cali_handle_ = nullptr;
    };


}