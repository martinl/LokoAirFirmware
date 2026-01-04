#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void bsp_uart_gnss_init(void);
void bsp_uart_debug_init(void);

void bsp_uart_gnss_byte_received(uint8_t byte);
void bsp_uart_debug_byte_received(uint8_t byte);

void bsp_uart_debug_write(uint8_t const *data, size_t size);
void bsp_uart_gnss_write(uint8_t const *data, size_t size);

#ifdef __cplusplus
}
#endif
