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

TEST_CASE("encode read holding register") {

    SUBCASE("Correct frame") {
        uint8_t slave_id = 1;
        uint16_t addr = 0x0000;
        uint16_t qty = 10;
        size_t cap = 8;
        size_t ret_len;
        uint8_t out_buff[cap];
        modbus_frame::ModbusFrameError ret = modbus_frame::encode_read_holding_register(
            slave_id,
            addr,
            qty,
            out_buff,
            cap,
            &ret_len
        );

        CHECK(ret == modbus_frame::ModbusFrameError::Ok);
        CHECK(ret_len == 8);

    }

    SUBCASE("Insuffiecient capacity") {
    uint8_t slave_id = 1;
    uint16_t addr = 0x0000;
    uint16_t qty = 10;
    size_t cap = 7;
    size_t ret_len = 0;
    uint8_t out_buff[cap];
    modbus_frame::ModbusFrameError ret = modbus_frame::encode_read_holding_register(
        slave_id,
        addr,
        qty,
        out_buff,
        cap,
        &ret_len
    );

    CHECK(ret == modbus_frame::ModbusFrameError::BufferTooSmall);
    CHECK(ret_len == 0);

    }

    SUBCASE("Correct frame") {
        uint8_t slave_id = 1;
        uint16_t addr = 0x0000;
        uint16_t qty = 0;
        size_t cap = 8;
        size_t ret_len = 0;
        uint8_t out_buff[cap];
        modbus_frame::ModbusFrameError ret = modbus_frame::encode_read_holding_register(
            slave_id,
            addr,
            qty,
            out_buff,
            cap,
            &ret_len
        );

        CHECK(ret == modbus_frame::ModbusFrameError::InvalidArgument);
        CHECK(ret_len == 0);

    }
}

    TEST_CASE("Decode read holding register") {

        SUBCASE("Short frame") {

            size_t frame_len = 6;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x01, 0x04, 0xf1, 0x8b};
            uint16_t expec_qty = 10;
            size_t out_cap = 11;
            uint16_t out_regs[expec_qty];


            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs,
                out_cap
            )
                == modbus_frame::ModbusFrameError::ShortFrame);
        }

        SUBCASE("Wrong CRC") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x02, 0x12, 0x34, 0xb4, 0x33};
            uint16_t expec_qty = 10;
            size_t out_cap = 11;
            uint16_t out_regs[expec_qty];

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs,
                out_cap
            )
                == modbus_frame::ModbusFrameError::BadCrc);

        }

        SUBCASE("Exception response") {

            size_t frame_len = 5;
            const uint8_t frame[frame_len] = {0x01, 0x83, 0x01, 0x80, 0xf0};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            uint16_t out_regs[expec_qty];

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs,
                out_cap
            )
                == modbus_frame::ModbusFrameError::ExceptionResponse);

        }

        SUBCASE("Wrong FC") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x02, 0x02, 0x80, 0xf0, 0xd8, 0x3c};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            uint16_t out_regs[expec_qty];

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs,
                out_cap
            )
                == modbus_frame::ModbusFrameError::BadFunctionCode);

        }

        SUBCASE("Byte mismatch") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x04, 0x80, 0xf0, 0x39, 0xc1};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            uint16_t out_regs[expec_qty];

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs,
                out_cap
            )
                == modbus_frame::ModbusFrameError::ByteCountMismatch);

        }

        SUBCASE("Happy path") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x02, 0x80, 0xf0, 0xd9, 0xc0};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            uint16_t out_regs[expec_qty];

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs,
                out_cap
            )
                == modbus_frame::ModbusFrameError::Ok);

        }

        SUBCASE("Buffer too small") {

            size_t frame_len = 9;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x04, 0x80, 0xf0, 0x12, 0x34, 0xde, 0xb7};
            uint16_t expec_qty = 2;
            size_t out_cap = 1;
            uint16_t out_regs[expec_qty];

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs,
                out_cap
            )
                == modbus_frame::ModbusFrameError::BufferTooSmall);

        }

    }