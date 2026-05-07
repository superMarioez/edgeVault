#pragma once

#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include <cstdint>
#include <cstddef>

namespace modbus_transport {

    class ModbusTransport {

        public:

            ModbusTransport(uart_port_t port, int tx_pin, int rx_pin, int rts_pin, uint32_t baud_rate);
            ~ModbusTransport();

            ModbusTransport(const ModbusTransport&) = delete;
            ModbusTransport& operator=(const ModbusTransport&) = delete;

            ModbusTransport(ModbusTransport&& other) noexcept;
            ModbusTransport& operator=(ModbusTransport&& other) noexcept;

            /**
             * @brief A method to send the modbus master request to a slave sensor and receiving it's response in an application layer owned response buffer.
             * 
             *  one blocking request/response transaction.
             * 
             * @param req               The modbus master request frame.
             * @param req_len           Request frame length.
             * @param resp              The manager task owned response buffer.
             * @param resp_cap          Maximum bytes the manager task can receive per transaction.
             * @param resp_len          The actual received bytes sent from the slave sensor.
             * @param timeout_ms        Maximum time for this blocking call to wait for a slave response.
             * 
             * @return ESP_OK on success, ESP_ERR_TIMEOUT if no bytes received in window, ESP_ERR_INVALID_ARG on null pointer / zero-length, ESP_FAIL on driver-level error.
             * 
             */
            esp_err_t transact(
                uint8_t* req,
                size_t req_len,
                uint8_t* resp,
                size_t resp_cap,
                size_t& resp_len,
                uint32_t timeout_ms
            );

        private:

            uart_port_t port_ = UART_NUM_MAX; // using UART_NUM_MAX as a sentinel value

            static constexpr size_t RX_BUF_SIZE = 512; // 512 is a reasonable number for back-ti-back transactions
            static constexpr size_t TX_BUF_SIZE = 0;
            static constexpr size_t UART_QUEUE_LEN = 0;
    };


}