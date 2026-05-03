/* RTU frame encode/decode + CRC */
#pragma once
#include <cstdint>
#include <cstddef>




namespace modbus_frame {

    inline constexpr const uint16_t REFLECTED_POLYNOMIAL = 0xA001;
    inline constexpr const uint8_t BIT_LENGTH = 8;
    inline constexpr const uint8_t FC_03 = 0x03;
    inline constexpr const uint8_t FC_EXCEPTION_83 = 0x83;
    inline constexpr const uint8_t MODBUS_RESPONSE_FRAME_OVERHEAD = 5;
    inline constexpr const uint8_t MODBUS_SLAVE_ID_BYTE_INDEX = 0;
    inline constexpr const uint8_t MODBUS_FUNCTION_CODE_BYTE_INDEX = 1;

    enum class ModbusFrameError : int32_t {
        Ok,
        BadCrc,
        ShortFrame,
        BadFunctionCode,
        ByteCountMismatch,
        ExceptionResponse,
        BufferTooSmall,
        InvalidArgument
    };


    uint16_t crc16_modbus(const uint8_t*, size_t);


    /**
     * @brief Packs the request frame following the standard modbus format
     * 
     * Runs a validation scheme on the buffer items to be populated before being packed into it,
     * calculates the CRC on the buffer's first 6 bytes
     * 
     * @param slave_id          the modbus slave device id
     * @param addr              the starting holding register address to be read
     * @param qty               number of registers to be read starting from addr
     * @param out_buffer        the output buffer containing the validated frame
     * @param out_capacity      capacity of the output buffer in bytes, must >= 8
     * @param out_len           number of bytes have been populated in the output buffer
     * 
     * @return ModbusFrameError::Ok on success, or a specific error code on validation failure.
     */
    ModbusFrameError encode_read_holding_register(
        const uint8_t,
        const uint16_t,
        const uint16_t,
        uint8_t*,
        const size_t,
        size_t*
    );

    /**
     * @brief Decodes a Modbus Read Holding Registers (FC03) response frame.
     * 
     * Runs a strict validation gauntlet on the incoming frame to ensure electrical 
     * integrity and protocol compliance before extracting the payload.
     * 
     * @param frame             Pointer to the raw bytes received from the UART.
     * @param frame_len         Total number of bytes in the received frame.
     * @param expected_quantity The number of registers originally requested by the master.
     * @param out_registers     Buffer to store the extracted 16-bit register values.
     * @param out_capacity      WARNING: The maximum number of REGISTERS this buffer can hold (NOT bytes!).
     * 
     * @return ModbusFrameError::Ok on success, or a specific error code on validation failure.
     */
    ModbusFrameError decode_read_holding_register(
        const uint8_t*,
        size_t,
        uint16_t,
        uint16_t*,
        size_t
    );

}