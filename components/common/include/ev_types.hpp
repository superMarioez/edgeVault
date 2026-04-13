#pragma once

#include <cstdint>
#include "freertos/event_groups.h"

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

inline constexpr SystemEvent operator|(SystemEvent a, SystemEvent b) {
    return static_cast<SystemEvent>(static_cast<EventBit_t>(a) | static_cast<EventBit_t>(b));
}

inline constexpr SystemEvent operator& (SystemEvent a, SystemEvent b) {
    return static_cast<SystemEvent>(static_cast<EventBit_t>(a) & static_cast<EventBit_t>(b));
}

inline constexpr EventBit_t to_bits(SystemEvent e) {
    return static_cast<EventBit_t>(e);
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
