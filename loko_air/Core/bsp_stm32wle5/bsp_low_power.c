#include "stm32wlxx_hal.h"
#include "stm32wlxx_ll_bus.h"
#include "stm32wlxx_ll_cortex.h"
#include "stm32wlxx_ll_exti.h"
#include "stm32wlxx_ll_pwr.h"
#include "stm32wlxx_ll_rcc.h"
#include "stm32wlxx_ll_system.h"
#include "stm32wlxx_ll_utils.h"
#include <bsp.h>

/* -------------------------------------------------------------------------- */

void bsp_lp_shutdown(bsp_wakeup_source_mask_list_t source_mask, size_t sleep_time_s) {
    LL_PWR_EnableBkUpAccess();
    __HAL_RCC_RTCAPB_CLK_ENABLE();

    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN1);
    HAL_PWR_DisableWakeUpPin(PWR_WAKEUP_PIN3);

    /* Clear all related wake-up flags*/
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_SB);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WPVD);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WRFBUSY);

    if (source_mask & WAKEUP_SOURCE_BUTTON_1_MASK) {
        HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN1);
    }

    if (source_mask & WAKEUP_SOURCE_5V_DETECT_MASK) {
        // 5V Detect pin - PB3
        if (bsp_gpio_is_usb_charger_connect() == false) {
            HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3_HIGH);
        } else {
            HAL_PWR_EnableWakeUpPin(PWR_WAKEUP_PIN3_LOW);
        }
    }

#if CONFIG_BSP_USE_RTC == 1
    if (source_mask & WAKEUP_SOURCE_TIMER_MASK) {
        bsp_rtc_setup_wakeup_timer(sleep_time_s);
    }
#endif  // CONFIG_BSP_USE_RTC == 1

    HAL_PWREx_DisableSRAMRetention();  // For Standby mode
    HAL_PWREx_EnterSHUTDOWNMode();     // Exit from here - it is MCU reboot
}

/* -------------------------------------------------------------------------- */

void bsp_lp_sleep(void) {
    __WFI();
}

/* -------------------------------------------------------------------------- */
