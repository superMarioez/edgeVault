#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "modbus_frame.hpp"

TEST_CASE("CRC16 Modbus Mathematical Verfication") {

    SUBCASE("Standard 6-byte frame") {

        uint8_t frame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0A};
        uint16_t result = modbus_frame::crc16_modbus(frame, sizeof(frame));
        CHECK(result == 0xCDC5);

    }

    SUBCASE("Empty buffer returns initial value") {

        uint8_t* frame = nullptr;
        uint16_t result = modbus_frame::crc16_modbus(frame, 0);
        CHECK(result == 0xFFFF);

    }

    SUBCASE("Single bye 0x01") {

        uint8_t single_byte[] = {0x01};
        uint16_t result = modbus_frame::crc16_modbus(single_byte, sizeof(single_byte));
        CHECK(result == 0x807E);

    }
}