#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

uint32_t bsp_adc_get_vbat_mv(void);
void bsp_adc_dma_handler(void);

#ifdef __cplusplus
}
#endif
