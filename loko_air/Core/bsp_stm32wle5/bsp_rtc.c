#include "bsp_rtc.h"
#include "bsp.h"

#include "stm32wlxx_hal.h"

/* -------------------------------------------------------------------------- */

// static
RTC_HandleTypeDef _rtc;

/* -------------------------------------------------------------------------- */
#define RTC_N_PREDIV_S 10
#define RTC_PREDIV_S   ((1 << RTC_N_PREDIV_S) - 1)
#define RTC_PREDIV_A   ((1 << (15 - RTC_N_PREDIV_S)) - 1)

void bsp_rtc_init(void) {
    _rtc.Instance = RTC;
    _rtc.Init.HourFormat = RTC_HOURFORMAT_24;
    _rtc.Init.AsynchPrediv = RTC_PREDIV_A;
    _rtc.Init.SynchPrediv = RTC_PREDIV_S;
    _rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    _rtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    _rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    _rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    _rtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
    _rtc.Init.BinMode = RTC_BINARY_ONLY;

    HAL_StatusTypeDef ret = HAL_RTC_Init(&_rtc);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }

    ret = HAL_RTCEx_SetSSRU_IT(&_rtc);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }

    RTC_AlarmTypeDef sAlarm = { 0 };
    sAlarm.BinaryAutoClr = RTC_ALARMSUBSECONDBIN_AUTOCLR_NO;
    sAlarm.AlarmTime.SubSeconds = 0x0;
    sAlarm.AlarmMask = RTC_ALARMMASK_NONE;
    sAlarm.AlarmSubSecondMask = RTC_ALARMSUBSECONDBINMASK_NONE;
    sAlarm.Alarm = RTC_ALARM_A;
    if (HAL_RTC_SetAlarm_IT(&_rtc, &sAlarm, 0) != HAL_OK) {}

    HAL_NVIC_SetPriority(RTC_Alarm_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(RTC_Alarm_IRQn);
}

/* -------------------------------------------------------------------------- */

static void _reinit_rtc_for_1s(void) {
    _rtc.Instance = RTC;
    _rtc.Init.HourFormat = RTC_HOURFORMAT_24;
    _rtc.Init.AsynchPrediv = 0x7F;
    _rtc.Init.SynchPrediv = 0xFF;
    _rtc.Init.OutPut = RTC_OUTPUT_DISABLE;
    _rtc.Init.OutPutRemap = RTC_OUTPUT_REMAP_NONE;
    _rtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
    _rtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
    _rtc.Init.OutPutPullUp = RTC_OUTPUT_PULLUP_NONE;
    _rtc.Init.BinMode = RTC_BINARY_NONE;

    HAL_RTC_DeInit(&_rtc);
    HAL_StatusTypeDef ret = HAL_RTC_Init(&_rtc);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }

    ret = HAL_RTCEx_SetSSRU_IT(&_rtc);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }
}

/* -------------------------------------------------------------------------- */

void HAL_RTC_MspInit(RTC_HandleTypeDef *rtc_handle) {
    if (rtc_handle->Instance == RTC) {
        if (LL_RCC_GetRTCClockSource() != LL_RCC_RTC_CLKSOURCE_LSE) {
            /* Update LSE configuration in Backup Domain control register */
            /* Requires to enable write access to Backup Domain if necessary */
            if (LL_PWR_IsEnabledBkUpAccess() != 1U) {
                LL_PWR_EnableBkUpAccess();

                while (LL_PWR_IsEnabledBkUpAccess() == 0U) {
                    // Wait for complete
                }
            }

            LL_RCC_ForceBackupDomainReset();
            LL_RCC_ReleaseBackupDomainReset();
            LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
            LL_RCC_LSE_Enable();

            while (LL_RCC_LSE_IsReady() != 1) {
                // Wait for complete
            }

            LL_RCC_SetRTCClockSource(LL_RCC_RTC_CLKSOURCE_LSE);
        }

        __HAL_RCC_RTC_ENABLE();
        __HAL_RCC_RTCAPB_CLK_ENABLE();
    }
}

/* -------------------------------------------------------------------------- */

void HAL_RTC_MspDeInit(RTC_HandleTypeDef *rtc_handle) {
    if (rtc_handle->Instance == RTC) {
        __HAL_RCC_RTC_DISABLE();
        __HAL_RCC_RTCAPB_CLK_DISABLE();

        HAL_NVIC_DisableIRQ(TAMP_STAMP_LSECSS_SSRU_IRQn);
        HAL_NVIC_DisableIRQ(RTC_Alarm_IRQn);
    }
}

/* -------------------------------------------------------------------------- */

void bsp_rtc_setup_wakeup_timer(uint32_t timeout_s) {
    _reinit_rtc_for_1s();
    /*  To configure the wake up timer to 4s the WakeUpCounter is set to 0x1FFF:
        RTC_WAKEUPCLOCK_RTCCLK_DIV = RTCCLK_Div16 = 16
        Wakeup Time Base = 16 /(~39.000KHz) = ~0,410 ms
        Wakeup Time = ~4s = 0,410ms  * WakeUpCounter
        ==> WakeUpCounter = ~4s/0,410ms = 9750 = 0x2616
    */
    HAL_RTCEx_DeactivateWakeUpTimer(&_rtc);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

    HAL_StatusTypeDef ret;
    ret = HAL_RTCEx_SetWakeUpTimer_IT(&_rtc, timeout_s, RTC_WAKEUPCLOCK_CK_SPRE_16BITS, 0);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }
}

/* -------------------------------------------------------------------------- */

bool bsp_rtc_is_wakeup_activated(void) {
    return (_rtc.Instance->CR & RTC_CR_WUTE);
}

/* -------------------------------------------------------------------------- */

void bsp_rtc_wakeup_deactivate(void) {
    HAL_RTCEx_DeactivateWakeUpTimer(&_rtc);
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
}

/* -------------------------------------------------------------------------- */

void bsp_rtc_store_write_reg(size_t reg, uint32_t value) {
    HAL_RTCEx_BKUPWrite(&_rtc, reg, value);
}

/* -------------------------------------------------------------------------- */

uint32_t bsp_rtc_store_read_reg(size_t reg) {
    return HAL_RTCEx_BKUPRead(&_rtc, reg);
}

/* -------------------------------------------------------------------------- */

uint32_t bsp_rtc_store_get_reg_count(void) {
    return RTC_BACKUP_NB;
}

/* -------------------------------------------------------------------------- */
