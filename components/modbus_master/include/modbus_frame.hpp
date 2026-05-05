/* RTU frame encode/decode + CRC */
#pragma once
#include <cstdint>
#include <cstddef>

namespace modbus_frame {

    inline constexpr uint16_t REFLECTED_POLYNOMIAL = 0xA001;
    inline constexpr uint8_t BIT_LENGTH = 8;
    inline constexpr uint8_t FC_03 = 0x03;
    inline constexpr uint8_t FC_EXCEPTION_83 = 0x83;
    inline constexpr uint8_t MODBUS_RESPONSE_FRAME_OVERHEAD = 5;
    inline constexpr uint8_t MODBUS_SLAVE_ID_BYTE_INDEX = 0;
    inline constexpr uint8_t MODBUS_FUNCTION_CODE_BYTE_INDEX = 1;


    /**
     * @brief Status codes returned by Modbus codec functions.
    */
    enum class ModbusFrameError : int32_t {
        Ok,                         ///< Operation completed successfully
        BadCrc,                     ///< Frame is corrupted; failed the cyclic redundancy check.
        ShortFrame,                 ///< Frame length is small to be mathematically valid.
        BadFunctionCode,            ///< Function code does not match expected or known values.
        ByteCountMismatch,          ///< Payload length contradicts the declared byte count.
        ExceptionResponse,          ///< Sensor rejected the request (high bit set on FC).
        MalformedFrame,             ///< Wire data is wrong
        BufferTooSmall,             ///< Caller-provided capacity is insufficient for the payload.
        InvalidArgument             ///< Caller-provided mathematically illegal input parameters.
    };

    enum class DataType {

        Int16,
        Uint16,
        Int32,
        Uint32,
        Float32

    };

    enum class ByteOrder {

        ABCD,                       ///< Big endian (Modbus normal).
        CDAB,                       ///< Word-swapped big endian - most common in industrial gear.
        BADC,                       ///< byte-swapped within each word.
        DCBA                        ///< full reverse, little endian.

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
        const uint8_t slave_id,
        const uint16_t addr,
        const uint16_t qty,
        uint8_t* out_buffer,
        const size_t out_capacity,
        size_t* out_len
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
        const uint8_t* frame,
        size_t frame_len,
        uint16_t expected_quantity,
        uint16_t* out_registers,
        size_t out_capacity
    );

    /**
     * @brief translates an exception response frame for the task layer to interpret it
     * 
     * Runs a strict validation guantlet on the exception response
     * 
     * @param frame                 Exception response frame
     * @param len                   Exception response length ( MUST be 5 bytes! )
     * @param out_exc_code          The transmitted error code (01 - 0B)
     * 
     * 
     * @return ModbusFrameError::Ok on success, or a specific error code on validation failure.
     */
    ModbusFrameError decode_exception_response(
        const uint8_t* frame,
        size_t len,
        uint8_t& out_exc_code
    );

    /**
     * @brief This function re-assembles the decoded register bytes according to the specific manufactured sensor byte order
     * 
     * 
     * 
     * @param words             Frame of the words to be decoded
     * @param word_count        Number of words to be decoded NOTE: Word count MUST be 1 for 16-bit types, and MUST be 2 for 32-bit values
     * @param type              Type of the value being decoded
     * @param order             According to this order, the bytes will be reassembeled NOTE: for 16-bit types, order is irrelevant
     * @param out               The transmitted decoded value
     * 
     * @return ModbusFrameError::Ok on success, or a specific error code on validation failure.
     */
    ModbusFrameError decode_value(
        const uint16_t* words,
        size_t word_count,
        DataType type,
        ByteOrder order,
        float& out
    );

}