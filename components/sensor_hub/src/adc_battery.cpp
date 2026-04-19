#include "adc_battery.hpp"
#include <cmath>

namespace sensorhub {

    AdcBattery::AdcBattery() {

        adc_oneshot_unit_init_cfg_t adc_cfg = {};
        adc_cfg.clk_src = ADC_RTC_CLK_SRC_DEFAULT;
        adc_cfg.ulp_mode = ADC_ULP_MODE_DISABLE;
        adc_cfg.unit_id = ADC_UNIT_1;

        ESP_ERROR_CHECK(adc_oneshot_new_unit(
            &adc_cfg,
            &adc_handle_
        ));

        adc_oneshot_chan_cfg_t ch_cfg = {
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT
        };

        ESP_ERROR_CHECK(adc_oneshot_config_channel(
            adc_handle_,
            static_cast<adc_channel_t>(CONFIG_EV_BATTERY_ADC_CHANNEL),
            &ch_cfg
        ));
        
        adc_cali_curve_fitting_config_t cali_cur_cfg = {
            .unit_id = ADC_UNIT_1,
            .chan = static_cast<adc_channel_t>(CONFIG_EV_BATTERY_ADC_CHANNEL),
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT
        };

        ESP_ERROR_CHECK(adc_cali_create_scheme_curve_fitting(
            &cali_cur_cfg,
            &cali_handle_
        ));
    }

    AdcBattery::~AdcBattery() {
        if (cali_handle_ != nullptr) adc_cali_delete_scheme_curve_fitting(cali_handle_);
        if (adc_handle_ != nullptr) adc_oneshot_del_unit(adc_handle_);
    }

    AdcBattery::AdcBattery(AdcBattery&& other) noexcept : adc_handle_(other.adc_handle_), cali_handle_(other.cali_handle_) {
        other.adc_handle_ = nullptr;
        other.cali_handle_ = nullptr;
    }

    AdcBattery& AdcBattery::operator=(AdcBattery&& other) noexcept {
        if (this == &other) return *this;

        if (cali_handle_ != nullptr) adc_cali_delete_scheme_curve_fitting(cali_handle_);
        if (adc_handle_ != nullptr) adc_oneshot_del_unit(adc_handle_);
        
        adc_handle_ = other.adc_handle_;
        cali_handle_ = other.cali_handle_;
        other.adc_handle_ = nullptr;
        other.cali_handle_ = nullptr;

        return *this;
    }

    esp_err_t AdcBattery::read_voltage_mv(int& out_mv) const {

        int raw_val = 0, raw_avg = 0;
        esp_err_t ret = ESP_FAIL;
        constexpr uint8_t SAMPLE_RATE = 8;

        for (uint8_t i = 0; i < SAMPLE_RATE; ++i) {
            ret = adc_oneshot_read(
                adc_handle_,
                static_cast<adc_channel_t>(CONFIG_EV_BATTERY_ADC_CHANNEL),
                &raw_val
            );
            if (ret != ESP_OK) return ret;
            raw_avg += raw_val;
        }

        raw_avg /= SAMPLE_RATE;

        ret = adc_cali_raw_to_voltage(
            cali_handle_,
            raw_avg,
            &out_mv
        );

        if (ret != ESP_OK) return ret;

        out_mv = (out_mv * CONFIG_EV_BATTERY_DIVIDER_RATIO_X10) / 10;

        return ret;
    }


}