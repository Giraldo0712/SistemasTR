#include "library_uart.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "esp_err.h"

void uart_comm_init(void)
{
    uart_config_t uart_config = {
        .baud_rate = UART_COMM_BAUD_RATE,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(
        uart_driver_install(
            UART_COMM_PORT,
            UART_COMM_BUF_SIZE * 2,
            UART_COMM_BUF_SIZE * 2,
            0,
            NULL,
            0
        )
    );

    ESP_ERROR_CHECK(
        uart_param_config(
            UART_COMM_PORT,
            &uart_config
        )
    );

    ESP_ERROR_CHECK(
        uart_set_pin(
            UART_COMM_PORT,
            UART_COMM_TXD,
            UART_COMM_RXD,
            UART_PIN_NO_CHANGE,
            UART_PIN_NO_CHANGE
        )
    );
}

int uart_comm_read_bytes(uint8_t *data,
                         size_t len,
                         TickType_t ticks_to_wait)
{
    return uart_read_bytes(
        UART_COMM_PORT,
        data,
        len,
        ticks_to_wait
    );
}

void uart_comm_write_str(const char *str)
{
    if (str == NULL) {
        return;
    }

    uart_write_bytes(
        UART_COMM_PORT,
        str,
        strlen(str)
    );
}

void uart_comm_write_fmt(const char *fmt, ...)
{
    if (fmt == NULL) {
        return;
    }

    char buffer[UART_COMM_BUF_SIZE];

    va_list args;

    va_start(args, fmt);

    vsnprintf(
        buffer,
        sizeof(buffer),
        fmt,
        args
    );

    va_end(args);

    uart_write_bytes(
        UART_COMM_PORT,
        buffer,
        strlen(buffer)
    );
}