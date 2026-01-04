#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    WAKEUP_SOURCE_BUTTON_1_MASK = 0x00000001,
    WAKEUP_SOURCE_TIMER_MASK = 0x00000002,
    WAKEUP_SOURCE_5V_DETECT_MASK = 0x00000004,
} bsp_wakeup_source_mask_list_t;

void bsp_lp_shutdown(bsp_wakeup_source_mask_list_t source_mask, size_t sleep_time_s);
void bsp_lp_sleep(void);

#ifdef __cplusplus
}
#endif
