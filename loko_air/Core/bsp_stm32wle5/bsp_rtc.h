#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdbool.h"

void bsp_rtc_init(void);
void bsp_rtc_setup_wakeup_timer(uint32_t timeout_s);
bool bsp_rtc_is_wakeup_activated(void);
void bsp_rtc_wakeup_deactivate(void);

void bsp_rtc_store_write_reg(size_t reg, uint32_t value);
uint32_t bsp_rtc_store_read_reg(size_t reg);
uint32_t bsp_rtc_store_get_reg_count(void);

#ifdef __cplusplus
}
#endif
