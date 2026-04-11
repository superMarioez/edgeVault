#pragma once

#include <cstdint>

namespace ev {

struct SensorReading {
    int64_t  timestamp_ms;
    uint8_t  source_id;
    uint16_t register_addr;
    float    value;
    uint8_t  quality;   // 0=good, 1=stale, 2=error
};

} // namespace ev
