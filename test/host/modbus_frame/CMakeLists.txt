cmake_minimum_required(VERSION 3.16)
project(modbus_frame_tests)

set(EDGEVAULT_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../..")

include_directories(
    "${EDGEVAULT_ROOT}/components/modbus_master/include"
)

add_executable(
    modbus_frame_tests
    test_modbus_frame.cpp
    "${EDGEVAULT_ROOT}/components/modbus_master/src/modbus_frame.cpp"
)