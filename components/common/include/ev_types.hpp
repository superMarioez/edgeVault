#pragma once

#include <cstdint>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_log.h"

namespace ev {

enum class SensorSource : uint8_t {
    Modbus,
    Lm75a,
    Ds3231Temp,
    Adc,
    Bme280
};

enum class Quality : uint8_t {
    Good,
    Stale,
    Error
};

enum class SystemEvent : EventBits_t {
    WifiConnected = (1 << 0),
    MqttConnected = (1 << 1),
    NtpSynced = (1 << 2),
    SensorError = (1 << 3),
    LowBattery = (1 << 4)
};

struct SensorInfo {
    const char* sensor;
    const char* unit;
};

inline constexpr SystemEvent operator|(SystemEvent a, SystemEvent b) {
    return static_cast<SystemEvent>(static_cast<EventBits_t>(a) | static_cast<EventBits_t>(b));
}

inline constexpr SystemEvent operator& (SystemEvent a, SystemEvent b) {
    return static_cast<SystemEvent>(static_cast<EventBits_t>(a) & static_cast<EventBits_t>(b));
}

inline constexpr EventBits_t to_bits(SystemEvent e) {
    return static_cast<EventBits_t>(e);
}

inline constexpr SensorInfo ssr_id_to_str(SensorSource ssr_src) {
    
    SensorInfo ssr_info = {};
    switch(ssr_src) {
        case SensorSource::Adc:
        ssr_info.sensor = "ADC";
        ssr_info.unit = "volts";
        break;
        
        case SensorSource::Bme280:
        ssr_info.sensor = "BME280";
        ssr_info.unit = "";
        break;
        
        case SensorSource::Ds3231Temp:
        ssr_info.sensor = "DS3231";
        ssr_info.unit = "";
        break;

        case SensorSource::Lm75a:
        ssr_info.sensor = "LM75A";
        ssr_info.unit = "Celsius";
        break;

        case SensorSource::Modbus:
        ssr_info.sensor = "MODBUS";
        ssr_info.unit = "";
        break;
    }

    return ssr_info;
}

inline constexpr const char* quality_to_str(Quality quality) {

    switch(quality) {
        case Quality::Good:
        return "GOOD";

        case Quality::Stale:
        return "STALE";

        case Quality::Error:
        return "ERROR";
    }

    return "UNKNOWN";
}

namespace config {
    static constexpr uint8_t SENSOR_QUEUE_DEPTH = 30;
    static constexpr BaseType_t CORE_0 = 0;
    static constexpr BaseType_t CORE_1 = 1;
    static constexpr size_t TASK_STACK_SIZE_DEFAULT = 4096;
}

struct SensorReading {
    int64_t  timestamp_ms_;
    float    value_;
    uint16_t register_addr_;
    SensorSource sensor_;
    Quality quality_;
};



} // namespace ev