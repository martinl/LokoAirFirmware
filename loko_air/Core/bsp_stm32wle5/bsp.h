#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

#include "bsp_adc.h"
#include "bsp_clocks.h"
#include "bsp_flash.h"
#include "bsp_gpio.h"
#include <bsp_battery.h>
#include <bsp_low_power.h>
#include <bsp_rtc.h>
#include <bsp_uart.h>

typedef enum {
    BSP_START_REASON_POWER_ON = 0,
    BSP_START_REASON_RESET,
    BSP_START_REASON_BUTTON,
    BSP_START_REASON_CHARGER,
    BSP_START_REASON_TIMER_ALARM,

    BSP_START_REASON_COUNT
} bsp_start_reason_t;

bsp_start_reason_t bsp_get_start_reason(void);

#define BSP_ASSERT(condition)                                   \
    do {                                                        \
        if ((condition) == false) {                             \
            bsp_fatal_error(0, #condition, __FILE__, __LINE__); \
        }                                                       \
    } while (0)

#define BSP_FATAL(code, note) bsp_fatal_error(code, note, __FILE__, __LINE__)

void bsp_init(void);
void bsp_soft_breakpoint(void);
bool bsp_is_debug_session(void);
void bsp_system_reset(void);
void bsp_disable_irq(void);
void bsp_enable_irq(void);

uint32_t bsp_get_ticks(void);
void bsp_delay_ms(uint32_t ms);

uint64_t bsp_get_uid64(void);

void bsp_fatal_error(size_t code, char *note, char *file, size_t line);

void bsp_launch_app(void);

#ifdef __cplusplus
}
#endif
