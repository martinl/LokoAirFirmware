#include <stdio.h>
#include <string.h>

#include "bsp.h"
#include "bsp_battery.h"

/* -------------------------------------------------------------------------- */

static uint32_t _battery_voltage = 4200;

/* -------------------------------------------------------------------------- */

static uint32_t _low_pass_filter(uint32_t input, uint32_t prev) {
    //  The filter's smoothing factor. This value must be between 0 and 1.
    const float ALPHA = 0.1f;
    return (uint32_t)((float)prev * (1.0f - ALPHA) + (float)input * ALPHA);
}

/* -------------------------------------------------------------------------- */

void bsp_battery_measure(void) {
    _battery_voltage = _low_pass_filter(bsp_adc_get_vbat_mv(), _battery_voltage);
}

/* -------------------------------------------------------------------------- */

uint32_t bsp_battery_get_voltage(void) {
    return _battery_voltage;
}

/* -------------------------------------------------------------------------- */
