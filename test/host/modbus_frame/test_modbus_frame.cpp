#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "modbus_frame.hpp"
#include <cstdint>
#include <array>

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

TEST_CASE("Frame encoder") {

    SUBCASE("Correct frame") {
        uint8_t slave_id = 1;
        uint16_t addr = 0x0000;
        uint16_t qty = 10;
        size_t cap = 8;
        size_t ret_len;
        std::array<uint8_t, 8> out_buff {};
        modbus_frame::ModbusFrameError ret = modbus_frame::encode_read_holding_register(
            slave_id,
            addr,
            qty,
            out_buff.data(),
            cap,
            &ret_len
        );

        CHECK(ret == modbus_frame::ModbusFrameError::Ok);
        CHECK(ret_len == 8);

        uint8_t expected_frame[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x0a, 0xc5, 0xcd};

        CHECK( std::memcmp(out_buff.data(), expected_frame, ret_len) == 0 );
        
    }

    SUBCASE("Insuffiecient capacity") {
    uint8_t slave_id = 1;
    uint16_t addr = 0x0000;
    uint16_t qty = 10;
    size_t cap = 7;
    size_t ret_len = 0;
    std::array<uint8_t, 7> out_buff {};
    modbus_frame::ModbusFrameError ret = modbus_frame::encode_read_holding_register(
        slave_id,
        addr,
        qty,
        out_buff.data(),
        cap,
        &ret_len
    );

    CHECK(ret == modbus_frame::ModbusFrameError::BufferTooSmall);
    CHECK(ret_len == 0);

    }

    SUBCASE("Zero registers required") {
        uint8_t slave_id = 1;
        uint16_t addr = 0x0000;
        uint16_t qty = 0;
        size_t cap = 8;
        size_t ret_len = 0;
        std::array<uint8_t, 8> out_buff {};
        modbus_frame::ModbusFrameError ret = modbus_frame::encode_read_holding_register(
            slave_id,
            addr,
            qty,
            out_buff.data(),
            cap,
            &ret_len
        );

        CHECK(ret == modbus_frame::ModbusFrameError::InvalidArgument);
        CHECK(ret_len == 0);

    }

    SUBCASE("Application layer sends a nulled buffer for output") {

        uint8_t slave_id = 1;
        uint16_t addr = 0x0000;
        uint16_t qty = 10;
        size_t cap = 8;
        size_t ret_len = 0;
        uint8_t* out_buff = nullptr;
        modbus_frame::ModbusFrameError ret = modbus_frame::encode_read_holding_register(
            slave_id,
            addr,
            qty,
            out_buff,
            cap,
            &ret_len
        );

        CHECK(ret == modbus_frame::ModbusFrameError::InvalidArgument);

    }
}

    TEST_CASE("Frame decoder") {

        SUBCASE("Short frame") {

            size_t frame_len = 6;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x01, 0x04, 0xf1, 0x8b};
            uint16_t expec_qty = 10;
            size_t out_cap = 11;
            std::array<uint16_t, 10> out_regs {};


            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            )
                == modbus_frame::ModbusFrameError::ShortFrame);
        }

        SUBCASE("Wrong CRC") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x02, 0x12, 0x34, 0xb4, 0x33};
            uint16_t expec_qty = 10;
            size_t out_cap = 11;
            std::array<uint16_t, 10> out_regs {};

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            )
                == modbus_frame::ModbusFrameError::BadCrc);

        }

        SUBCASE("Exception response") {

            size_t frame_len = 5;
            const uint8_t frame[frame_len] = {0x01, 0x83, 0x01, 0x80, 0xf0};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            std::array<uint16_t, 1> out_regs {};

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            )
                == modbus_frame::ModbusFrameError::ExceptionResponse);

        }

        SUBCASE("Wrong FC") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x02, 0x02, 0x80, 0xf0, 0xd8, 0x3c};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            std::array<uint16_t, 1> out_regs {};

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            )
                == modbus_frame::ModbusFrameError::BadFunctionCode);

        }

        SUBCASE("Byte mismatch") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x04, 0x80, 0xf0, 0x39, 0xc1};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            std::array<uint16_t, 1> out_regs {};

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            )
                == modbus_frame::ModbusFrameError::ByteCountMismatch);

        }

        SUBCASE("Happy path") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x02, 0x80, 0xf0, 0xd9, 0xc0};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            std::array<uint16_t, 1> out_regs {};

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            )
                == modbus_frame::ModbusFrameError::Ok);

        }

        SUBCASE("Buffer too small") {

            size_t frame_len = 9;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x04, 0x80, 0xf0, 0x12, 0x34, 0xde, 0xb7};
            uint16_t expec_qty = 2;
            size_t out_cap = 1;
            std::array<uint16_t, 2> out_regs {};

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            )
                == modbus_frame::ModbusFrameError::BufferTooSmall);
        }

            SUBCASE("Trailing garbage") {

            size_t frame_len = 8;
            uint8_t frame[frame_len] = {0x01, 0x03, 0x02, 0x80, 0xf0, 0xff, 0x00, 0x00};
            uint16_t expec_qty = 1;
            size_t out_cap = 11;
            std::array<uint16_t, 1> out_regs {};

            // Dynamically calculate the perfect CRC for the mutated 6-byte payload
            // so it successfully sneaks past the CRC check in the validation guantlet.
            uint16_t dynamic_crc = modbus_frame::crc16_modbus(frame, frame_len - 2);
            frame[6] = dynamic_crc & 0xFF;
            frame[7] = (dynamic_crc >> 8) & 0xFF;

            CHECK( modbus_frame::ModbusFrameError::MalformedFrame == 
            modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            ));
        }

        SUBCASE("Sensor replies with more data than the expected (Expected quantity vs. Byte count)") {

            size_t frame_len = 7;
            const uint8_t frame[frame_len] = {0x01, 0x03, 0x02, 0x80, 0xf0, 0xd9, 0xc0};
            uint16_t expec_qty = 50;
            size_t out_cap = 11;
            std::array<uint16_t, 50> out_regs {};

            CHECK(modbus_frame::decode_read_holding_register(
                frame,
                frame_len,
                expec_qty,
                out_regs.data(),
                out_cap
            )
                == modbus_frame::ModbusFrameError::ByteCountMismatch);
        }
    }

    TEST_CASE("Exception response decoder") {

        SUBCASE("Valid exception response") {

            const uint8_t frame[5] = {0x01, 0x83, 0x02, 0xc0, 0xf1};
            size_t len = 5;
            uint8_t exception_code = 0;

            CHECK(
                modbus_frame::ModbusFrameError::Ok ==
                modbus_frame::decode_exception_response(frame, len, exception_code)
            );

            CHECK( 0x02 == exception_code );

        }

        SUBCASE("Wrong length ( < 5)") {

            const uint8_t frame[5] = {0x01, 0x83, 0x02, 0xc0, 0xf1};
            size_t len = 4;
            uint8_t exception_code = 0;

            CHECK(
                modbus_frame::ModbusFrameError::ShortFrame ==
                modbus_frame::decode_exception_response(frame, len, exception_code)
            );

        }

        SUBCASE("Wrong length ( > 5)") {

            const uint8_t frame[5] = {0x01, 0x83, 0x02, 0xc0, 0xf1};
            size_t len = 6;
            uint8_t exception_code = 0;

            CHECK(
                modbus_frame::ModbusFrameError::MalformedFrame ==
                modbus_frame::decode_exception_response(frame, len, exception_code)
            );

        }

        SUBCASE("Bad crc") {

            const uint8_t frame[5] = {0x01, 0x83, 0x02, 0xc0, 0xf2};
            size_t len = 5;
            uint8_t exception_code = 0;

            CHECK(
                modbus_frame::ModbusFrameError::BadCrc ==
                modbus_frame::decode_exception_response(frame, len, exception_code)
            );

        }

        SUBCASE("Function code high bit not set") {

            const uint8_t frame[5] = {0x01, 0x03, 0x02, 0xa1, 0x31};
            size_t len = 5;
            uint8_t exception_code = 0;

            CHECK(
                modbus_frame::ModbusFrameError::BadFunctionCode ==
                modbus_frame::decode_exception_response(frame, len, exception_code)
            );

        }

    }

    TEST_CASE("Value decoder") {

        SUBCASE("Uint16") {

        uint16_t words[] = {0x0001};
        size_t word_count = 1;
        modbus_frame::DataType type = modbus_frame::DataType::Uint16;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::ABCD; // Irrelevant
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }

        SUBCASE("Int16, +ve target value") {

        uint16_t words[] = {0x0001};
        size_t word_count = 1;
        modbus_frame::DataType type = modbus_frame::DataType::Int16;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::ABCD; // Irrelevant
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }

        
        SUBCASE("Int16, -ve target value") {

        uint16_t words[] = {0xffff};
        size_t word_count = 1;
        modbus_frame::DataType type = modbus_frame::DataType::Int16;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::ABCD; // Irrelevant
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( -1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Uint32, Big endian (ABCD)") {

        uint16_t words[] = {0x0000, 0x0001};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Uint32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::ABCD;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Uint32, Word-swapped big endian (CDAB)") {

        uint16_t words[] = {0x0001, 0x0000};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Uint32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::CDAB;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Uint32, Bytes swapped within the word (BADC)") {

        uint16_t words[] = {0x0000, 0x0100};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Uint32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::BADC;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Uint32, Full reverse little endian (DCBA)") {

        uint16_t words[] = {0x0100, 0x0000};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Uint32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::DCBA;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }



        SUBCASE("Int32, Big endian (ABCD)") {

        uint16_t words[] = {0x0000, 0x0001};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Int32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::ABCD;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Int32, Word-swapped big endian (CDAB)") {

        uint16_t words[] = {0x0001, 0x0000};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Int32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::CDAB;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Int32, Bytes swapped within the word (BADC)") {

        uint16_t words[] = {0x0000, 0x0100};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Int32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::BADC;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Int32, Full reverse little endian (DCBA)") {

        uint16_t words[] = {0x0100, 0x0000};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Int32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::DCBA;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Float32, Big endian (ABCD)") {

        uint16_t words[] = {0x3f80, 0x0000};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Float32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::ABCD;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Float32, Word-swapped big endian (CDAB)") {

        uint16_t words[] = {0x0000, 0x3f80};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Float32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::CDAB;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Float32, Bytes swapped within the word (BADC)") {

        uint16_t words[] = {0x803f, 0x0000};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Float32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::BADC;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


        SUBCASE("Float32, Full reverse little endian (DCBA)") {

        uint16_t words[] = {0x0000, 0x803f};
        size_t word_count = 2;
        modbus_frame::DataType type = modbus_frame::DataType::Float32;
        modbus_frame::ByteOrder order = modbus_frame::ByteOrder::DCBA;
        float out = 0.0f;

        modbus_frame::ModbusFrameError ret = modbus_frame::decode_value(words, word_count, type, order, out);

        CHECK( 1.0f == out );
        CHECK(modbus_frame::ModbusFrameError::Ok == ret);

        }


    }