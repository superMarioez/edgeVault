/* RAII wrappers over the UART driver + DE/RE pin */

#include "modbus_transport.hpp"
#include "esp_log.h"


namespace modbus_transport {

    static constexpr const char* TAG = "modbus_transport";


    ModbusTransport::ModbusTransport(uart_port_t port, int tx_pin, int rx_pin, int rts_pin, uint32_t baud_rate)
    {

        uart_config_t cfg = {};
        cfg.baud_rate = baud_rate;
        cfg.data_bits = UART_DATA_8_BITS;
        cfg.parity = UART_PARITY_DISABLE;
        cfg.stop_bits = UART_STOP_BITS_1;
        cfg.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        cfg.source_clk = UART_SCLK_DEFAULT;


        ESP_ERROR_CHECK(uart_driver_install(
            port,
            RX_BUF_SIZE,
            TX_BUF_SIZE,
            UART_QUEUE_LEN,
            nullptr,
            0
        ));

        ESP_ERROR_CHECK(uart_param_config(port, &cfg));
        ESP_ERROR_CHECK(uart_set_pin(port, tx_pin, rx_pin, rts_pin, UART_PIN_NO_CHANGE));
        ESP_ERROR_CHECK(uart_set_mode(port, UART_MODE_RS485_HALF_DUPLEX));
        ESP_ERROR_CHECK(uart_set_rx_timeout(port, 3));

        port_ = port;
    }

    ModbusTransport::~ModbusTransport() {

        if (port_ != UART_NUM_MAX) {
            esp_err_t ret = uart_driver_delete(port_);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "uart_driver_delete failed: %s", esp_err_to_name(ret));
            }
            port_ = UART_NUM_MAX;
        }

    }

    ModbusTransport::ModbusTransport(ModbusTransport&& other) noexcept : port_(other.port_) {
        other.port_ = UART_NUM_MAX;
    }

    ModbusTransport& ModbusTransport::operator=(ModbusTransport&& other) noexcept {
        if (this == &other) return *this;

        if (port_ != UART_NUM_MAX) uart_driver_delete(port_);

        port_ = other.port_;
        other.port_ = UART_NUM_MAX;
        return *this;
    }

    esp_err_t ModbusTransport::transact(
        uint8_t* req,
        size_t req_len,
        uint8_t* resp,
        size_t resp_cap,
        size_t& resp_len,
        uint32_t timeout_ms
    )
    {
        
    }




}