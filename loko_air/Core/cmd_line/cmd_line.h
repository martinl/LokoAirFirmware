#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stddef.h>
#include <stdint.h>

#define CONSOLE(str) (cmd_output(str, sizeof(str) - 1))

#define CONSOLE_EOL "\r\n"

void cmd_line_receive(uint8_t byte);
void cmd_output(char const *string, size_t len);

#if BUILD_CONF_UART_ECHO == 1
bsp_uart_write(&byte, 1);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
