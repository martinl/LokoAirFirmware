#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void bsp_battery_measure(void);
uint32_t bsp_battery_get_voltage(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
