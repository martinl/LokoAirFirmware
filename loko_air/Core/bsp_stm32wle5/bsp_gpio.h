#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

void bsp_gpio_init(void);

bool bsp_gpio_is_button_pressed(void);
bool bsp_gpio_is_charger_high(void);
bool bsp_gpio_is_usb_charger_connect(void);
void bsp_gpio_led_on(void);
void bsp_gpio_led_off(void);
void bsp_gpio_led_toggle(void);
void bsp_gpio_gnss_on(void);
void bsp_gpio_gnss_off(void);
void bsp_gpio_gnss_wakeup_enter(void);
void bsp_gpio_gnss_wakeup_leave(void);
void bsp_gpio_vbat_measure_on(void);
void bsp_gpio_vbat_measure_off(void);

#ifdef __cplusplus
}
#endif
