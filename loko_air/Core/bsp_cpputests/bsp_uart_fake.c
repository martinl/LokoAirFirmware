#include "bsp.h"
#include <stdio.h>
#include <string.h>

static char _out_buffer[1024 * 10] = { 0 };
static size_t volatile _out_buffer_index = 0;

#define MIN(a, b) ((a) > (b) ? (b) : (a))

void bsp_uart_debug_write(uint8_t const *data, size_t size) {
    size_t copy_size = MIN(size, (sizeof(_out_buffer) - _out_buffer_index));

    memcpy(&_out_buffer[_out_buffer_index], data, copy_size);
    _out_buffer_index += copy_size;
#if DEBUG_BUILD == 1
    printf("%*s", size, data);
#endif
}

size_t bsp_uart_debug_get_buffer(char *out_data, size_t size) {
    size_t copy_size = MIN(size, _out_buffer_index);

    memcpy(out_data, _out_buffer, copy_size);

    return copy_size;
}

void bsp_uart_debug_drop_buffer(void) {
    memset(_out_buffer, 0, sizeof(_out_buffer));
    _out_buffer_index = 0;
}
