#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#if CONFIG_LOKO_AIR == 1
#    include "bsp_loko_air_v2_board_.h"
#elif CONFIG_LOKO_AIR_SEEED_E5 == 1
#    include "bsp_loko_air_v2_seeed_e5_board_.h"
#else
#    error please define board
#endif

#ifdef __cplusplus
}
#endif
