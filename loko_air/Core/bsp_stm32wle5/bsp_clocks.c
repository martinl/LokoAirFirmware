#include <bsp.h>

#include "stm32wlxx_ll_rcc.h"
#include "stm32wlxx_ll_utils.h"

/* -------------------------------------------------------------------------- */

typedef struct {
    uint32_t core_hz;
    uint32_t msi_range;
    uint32_t flash_latency;
} bsp_cpu_clock_config_t;

/* -------------------------------------------------------------------------- */
// Wait states (WS)         HCLK3 (MHz)
// (access)             VCORE range 1       VCORE range 2
// 0 WS (1 HCLK cycle)      ≤ 18              ≤ 6
// 1 WS (2 HCLK cycles)     ≤ 36              ≤ 12
// 2 WS (3 HCLK cycles)     ≤ 48              ≤ 16

static const bsp_cpu_clock_config_t CLOCK_LIST[] = {
    [BSP_CLOCK_CORE_100_KHZ] = {
        .core_hz = 100 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_0,
        .flash_latency = LL_FLASH_LATENCY_0,
    },
    [BSP_CLOCK_CORE_200_KHZ] = {
        .core_hz = 200 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_1,
        .flash_latency = LL_FLASH_LATENCY_0,
    },
    [BSP_CLOCK_CORE_400_KHZ] = {
        .core_hz = 400 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_2,
        .flash_latency = LL_FLASH_LATENCY_0,
    },
    [BSP_CLOCK_CORE_800_KHZ] = {
        .core_hz = 800 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_3,
        .flash_latency = LL_FLASH_LATENCY_0,
    },
    [BSP_CLOCK_CORE_1_MHZ] = {
        .core_hz = 1 * 1000 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_4,
        .flash_latency = LL_FLASH_LATENCY_0,
    },
    [BSP_CLOCK_CORE_2_MHZ] = {
        .core_hz = 2 * 1000 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_5,
        .flash_latency = LL_FLASH_LATENCY_0,
    },
    [BSP_CLOCK_CORE_4_MHZ] = {
        .core_hz = 4 * 1000 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_6,
        .flash_latency = LL_FLASH_LATENCY_0,
    },
    [BSP_CLOCK_CORE_8_MHZ] = {
        .core_hz = 8 * 1000 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_7,
        .flash_latency = LL_FLASH_LATENCY_1,
    },
    [BSP_CLOCK_CORE_16_MHZ] = {
        .core_hz = 16 * 1000 * 1000UL,
        .msi_range = LL_RCC_MSIRANGE_8,
        .flash_latency = LL_FLASH_LATENCY_2,
    },
};

/* -------------------------------------------------------------------------- */

static void _clock_up_to_switch(bsp_clock_list_t freq_id) {
    bsp_cpu_clock_config_t const *const CLOCK_CONFIG = &CLOCK_LIST[freq_id];

    LL_FLASH_SetLatency(CLOCK_CONFIG->flash_latency);
    LL_RCC_MSI_SetRange(CLOCK_CONFIG->msi_range);
    LL_SetSystemCoreClock(CLOCK_CONFIG->core_hz);
    HAL_StatusTypeDef ret = HAL_InitTick(TICK_INT_PRIORITY);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }
}

/* -------------------------------------------------------------------------- */

static void _clock_down_to_switch(bsp_clock_list_t freq_id) {
    bsp_cpu_clock_config_t const *const CLOCK_CONFIG = &CLOCK_LIST[freq_id];

    LL_RCC_MSI_SetRange(CLOCK_CONFIG->msi_range);
    LL_FLASH_SetLatency(CLOCK_CONFIG->flash_latency);
    LL_SetSystemCoreClock(CLOCK_CONFIG->core_hz);
    HAL_StatusTypeDef ret = HAL_InitTick(TICK_INT_PRIORITY);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }
}

/* -------------------------------------------------------------------------- */

void bsp_clock_init(void) {
    bsp_cpu_clock_config_t const *const CLOCK_CONFIG = &CLOCK_LIST[DEFAULT_FREQ];

    LL_FLASH_SetLatency(CLOCK_CONFIG->flash_latency);

    while (LL_FLASH_GetLatency() != CLOCK_CONFIG->flash_latency) {
        // Wait for complete
    }

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1);
    LL_RCC_MSI_Enable();

    /* Wait till MSI is ready */
    while (LL_RCC_MSI_IsReady() != 1) {
        // Wait for complete
    }

    LL_RCC_MSI_EnableRangeSelection();
    LL_RCC_MSI_SetRange(CLOCK_CONFIG->msi_range);
    LL_RCC_MSI_SetCalibTrimming(0);
    LL_PWR_EnableBkUpAccess();
    LL_RCC_LSE_SetDriveCapability(LL_RCC_LSEDRIVE_LOW);
    LL_RCC_LSE_Enable();

    /* Wait till LSE is ready */
    while (LL_RCC_LSE_IsReady() != 1) {
        // Wait for complete
    }

    LL_RCC_MSI_EnablePLLMode();
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_MSI);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_MSI) {
        // Wait for complete
    }

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAHB3Prescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_1);
    /* Update CMSIS variable (which can be updated also through SystemCoreClockUpdate function) */
    LL_SetSystemCoreClock(CLOCK_CONFIG->core_hz);

    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE2);

    /* Update the time base */
    HAL_StatusTypeDef ret = HAL_InitTick(TICK_INT_PRIORITY);

    if (ret != HAL_OK) {
        BSP_FATAL(ret, NULL);
    }
}

/* -------------------------------------------------------------------------- */

void bsp_clock_switch(bsp_clock_list_t freq) {
    if (bsp_clock_cpu_core_freq_hz() > freq) {
        _clock_down_to_switch(freq);
    } else if (bsp_clock_cpu_core_freq_hz() < freq) {
        _clock_up_to_switch(freq);
    }
#if 0
    LOG_DEBUG("Clock Switched to %ldMHz", bsp_clock_cpu_core_freq_hz() / 1000000UL);
#endif
}

/* -------------------------------------------------------------------------- */

uint32_t bsp_clock_cpu_core_freq_hz(void) {
    return SystemCoreClock;
}
