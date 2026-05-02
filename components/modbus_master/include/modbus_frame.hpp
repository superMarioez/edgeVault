/* RTU frame encode/decode + CRC */
#pragma once
#include <cstdint>
#include <cstddef>




namespace modbus_frame {

    inline constexpr const uint16_t REFLECTED_POLYNOMIAL = 0xA001;
    inline constexpr const uint8_t BIT_LENGTH = 8;

    enum class ModbusFrameError : int32_t {
        Ok,
        BadCrc,
        ShortFrame,
        BadFunctionCode,
        ByteCountMismatch,
        ExceptionResponse,
        BufferTooSmall
    };


    uint16_t crc16_modbus(const uint8_t*, size_t);

}