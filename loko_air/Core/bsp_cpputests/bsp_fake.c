#include "bsp.h"

uint32_t _fake_ticks_ms = 0;

uint32_t bsp_get_ticks(void) {
    return _fake_ticks_ms;
}

void bsp_fake_forward_ticks_ms(uint32_t ms) {
    _fake_ticks_ms += ms;
}

void bsp_system_reset(void) {
}

uint64_t bsp_get_uid64(void) {
    return 0x1234567890ABCDEFLL;
}

void bsp_delay_ms(uint32_t ms) {
    (void)ms;
    return;
}