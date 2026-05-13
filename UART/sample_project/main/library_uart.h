#ifndef LIBRARY_UART_H
#define LIBRARY_UART_H

#include <stddef.h>
#include "driver/uart.h"
#include "freertos/FreeRTOS.h"

// UART0 -> USB serial integrado ESP32
#define UART_COMM_PORT        UART_NUM_0

// Pines por defecto:
// TX -> GPIO1
// RX -> GPIO3
#define UART_COMM_TXD         UART_PIN_NO_CHANGE
#define UART_COMM_RXD         UART_PIN_NO_CHANGE

#define UART_COMM_BAUD_RATE   115200
#define UART_COMM_BUF_SIZE    256

void uart_comm_init(void);

int uart_comm_read_bytes(uint8_t *data,
                         size_t len,
                         TickType_t ticks_to_wait);

void uart_comm_write_str(const char *str);

void uart_comm_write_fmt(const char *fmt, ...);

#endif