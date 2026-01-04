/**
 ******************************************************************************
 * @file
 * @author  DKrasutski@gmail.com
 * @version V1.0
 * @date    22.10.2017
 * @brief   This file contains all the functions prototypes for the HAL
 *          module driver.
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2016 Denis Krasutski </center></h2>
 *
 ******************************************************************************
 */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "compiler_abstraction.h"
#if CONFIG_LOKO_CPPUTEST == 0
#    define LOG_ENABLED_COLOR (1U)
#endif

#if (CONFIG_BOOTLOADER_BUILD == 0)
#    define LOG_ENABLED (1U)
#else
#    define LOG_ENABLED (0U)
#endif  // DEBUIG_BUILD == 0

#define START_ADDR (APP_START_ADDR)

#if (CONFIG_LOKO_AIR + CONFIG_LOKO_AIR_SEEED_E5 + CONFIG_LOKO_CPPUTEST) != 1U
#    error error build config, please use: CONFIG_LOKO_AIR or CONFIG_LOKO_CPPUTEST
#endif

#include "log_/log_.h"

#ifdef __cplusplus
}
#endif /* __cplusplus */
