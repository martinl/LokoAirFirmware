#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    BSP_CLOCK_CORE_100_KHZ,
    BSP_CLOCK_CORE_200_KHZ,
    BSP_CLOCK_CORE_400_KHZ,
    BSP_CLOCK_CORE_800_KHZ,
    BSP_CLOCK_CORE_1_MHZ,
    BSP_CLOCK_CORE_2_MHZ,
    BSP_CLOCK_CORE_4_MHZ,
    BSP_CLOCK_CORE_8_MHZ,
    BSP_CLOCK_CORE_16_MHZ,  // If you need frequency more than 16MHz do not forget setup
                            // LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE1)
} bsp_clock_list_t;

#define DEFAULT_FREQ (BSP_CLOCK_CORE_4_MHZ)

void bsp_clock_init(void);
void bsp_clock_switch(bsp_clock_list_t freq);
uint32_t bsp_clock_cpu_core_freq_hz(void);

#ifdef __cplusplus
}
#endif
