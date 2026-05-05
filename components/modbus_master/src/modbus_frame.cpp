/* CRC-16 + request/response framing */
#include "modbus_frame.hpp"
#include <cstring>


namespace modbus_frame {

    static constexpr void reorder_bytes(const ByteOrder& order, const uint16_t* words, uint32_t& reordered_bytes) {
        
        uint32_t a = static_cast<uint32_t>(words[0] >> 8 & 0x000000FF);
        uint32_t b = static_cast<uint32_t>(words[0] & 0x000000FF);
        uint32_t c = static_cast<uint32_t>(words[1] >> 8 & 0x000000FF);
        uint32_t d = static_cast<uint32_t>(words[1] & 0x000000FF);

        switch(order) {

            // Big endian, Modbus normal
            case ByteOrder::ABCD: {
                reordered_bytes = (a << 24) | (b << 16) | (c << 8) | d;
                break;
            }

            // Swapped big endian
            case ByteOrder::CDAB:
                reordered_bytes = (c << 24) | (d << 16) | (a << 8) | b;
                break;

            // Bytes swapped within the word
            case ByteOrder::BADC: {
                reordered_bytes = (b << 24) | (a << 16) | (d << 8) | c;
                break;
                
            }

            // full reverse, little endian
            case ByteOrder::DCBA: {
                reordered_bytes = (d << 24) | (c << 16) | (b << 8) | a;
                break;
                
            }

            default:
                reordered_bytes = 0;
                break;
        }
    }

    static void dispatch_type(const DataType& type, const uint32_t& raw_bits, float& output) {
        if ( type == DataType::Float32 ) {
            memcpy(&output, &raw_bits, sizeof(float));
        }
        else if (type == DataType::Uint32) {
            output = static_cast<float>(raw_bits);
        }
        else {
            output = static_cast<float>(static_cast<int32_t>(raw_bits));
        }
    }


    uint16_t crc16_modbus(const uint8_t* data, size_t len) {

        uint16_t crc = 0xffff;
        
        for (size_t i = 0; i < len; ++i) {

            crc ^= data[i];
            for (int bit = 0; bit < BIT_LENGTH; ++bit) {
                if (crc & 0x0001) crc = (crc >> 1) ^ REFLECTED_POLYNOMIAL;
                else crc >>= 1;
            }
        }

        return crc;

    }

    ModbusFrameError encode_read_holding_register(
        const uint8_t slave_id,
        const uint16_t addr,
        const uint16_t qty,
        uint8_t* out_buffer,
        const size_t out_capacity,
        size_t* out_len
    )

    {

        // validate the output length is not nullptr
        if (out_len != nullptr) *out_len = 0;
        else return ModbusFrameError::InvalidArgument;

        // validate the number of registers to be read
        if (qty <= 0 || qty > 125) return ModbusFrameError::InvalidArgument;
        
        // validate the buffer capacity >= 8
        if (out_capacity < 8) return ModbusFrameError::BufferTooSmall;

        // validate the output buffer
        if (out_buffer == nullptr) return ModbusFrameError::InvalidArgument;

        out_buffer[MODBUS_SLAVE_ID_BYTE_INDEX] = slave_id;
        out_buffer[MODBUS_FUNCTION_CODE_BYTE_INDEX] = FC_03;

        out_buffer[2] = (addr >> 8) & 0xFF;
        out_buffer[3] = addr & 0xFF;

        out_buffer[4] = (qty >> 8) & 0xFF;
        out_buffer[5] = qty & 0xFF;

        uint16_t crc = crc16_modbus(out_buffer, 6);
        out_buffer[6] = crc & 0xFF;
        out_buffer[7] = (crc >> 8) & 0xFF;

        *out_len = 8;

        return ModbusFrameError::Ok;
    }

    ModbusFrameError decode_read_holding_register(
        const uint8_t* frame,
        size_t frame_len,
        uint16_t expected_quantity,
        uint16_t* out_registers,
        size_t out_capacity
    )

    {
        /* slave id validation is for the application level caller to verify */

        /* Frame length validation (a frame physically cannot exist if it's less than 5 bytes)*/
        if (frame_len < 5) return ModbusFrameError::ShortFrame;
        
        /* CRC Validation */
        uint16_t calc_crc = crc16_modbus(frame, frame_len - 2);
        uint16_t crc = static_cast<uint16_t>((frame[frame_len - 1] << 8) & 0xFF00) |
                       static_cast<uint16_t>(frame[frame_len - 2] & 0x00FF);
        if (calc_crc != crc) return ModbusFrameError::BadCrc;

        /* Function code validation */
        if (frame[MODBUS_FUNCTION_CODE_BYTE_INDEX] == FC_EXCEPTION_83) return ModbusFrameError::ExceptionResponse;
        if (frame[MODBUS_FUNCTION_CODE_BYTE_INDEX] != FC_03) return ModbusFrameError::BadFunctionCode;
        

        /* If it's not an exception response, then it's a normal FD03 read */
        if (frame_len < 7) return ModbusFrameError::ShortFrame;

        /* Byte count validation */
        if (frame[2] != expected_quantity * 2)
            return ModbusFrameError::ByteCountMismatch;
        
        if (frame[2] != frame_len - MODBUS_RESPONSE_FRAME_OVERHEAD) return ModbusFrameError::ByteCountMismatch;

        // Safety check for not overwriting adjacent RAM addresses
        if (expected_quantity > out_capacity) return ModbusFrameError::BufferTooSmall;
        for (size_t i = 0; i < expected_quantity; ++i) {
            
            uint16_t hi_byte = frame[3 + (i * 2)] << 8;
            uint16_t low_byte = frame[4 + (i * 2)];

            out_registers[i] = hi_byte | low_byte;
            
        }

        return ModbusFrameError::Ok;
    }

    ModbusFrameError decode_exception_response(
        const uint8_t* frame,
        size_t len,
        uint8_t& out_exc_code
    )
    
    {

        if (len < 5) return ModbusFrameError::ShortFrame;
        if (len > 5) return ModbusFrameError::InvalidArgument;

        uint16_t calc_crc = crc16_modbus(frame, len - 2);
        uint16_t crc = (static_cast<uint16_t>(frame[len - 1]) << 8) & 0xFF00 |
                        static_cast<uint16_t>(frame[len - 2]) & 0x00FF;

        if (crc != calc_crc) return ModbusFrameError::BadCrc;
        
        if ( (frame[1] & 0x80) == 0 ) return ModbusFrameError::BadFunctionCode;

        out_exc_code = frame[2];

        return ModbusFrameError::Ok;

    }

    ModbusFrameError decode_value(
        const uint16_t* words,
        size_t word_count,
        DataType type,
        ByteOrder order,
        float& out
    )
    
    {

        if ( type == DataType::Int16 || type == DataType::Uint16 ) {
            // Word count MUST be 1
            if ( word_count != 1 ) return ModbusFrameError::InvalidArgument;

            if (type == DataType::Int16) out = static_cast<float>(static_cast<int16_t>(words[0]));
            else out = static_cast<float>(words[0]);

        }

        else {
            // Word count MUST be 2
            if ( word_count != 2 ) return ModbusFrameError::InvalidArgument;

            // here, the type is a 32 bit unsigned/signed int or a float
            // re-order first
            uint32_t raw_bits = 0;
            reorder_bytes(order, words, raw_bits);
            
            // dispatch according to the type
            dispatch_type(type, raw_bits, out);
        }

        return ModbusFrameError::Ok;

    }


}