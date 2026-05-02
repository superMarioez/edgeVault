/* CRC-16 + request/response framing */
#include "modbus_frame.hpp"



namespace modbus_frame {


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


}