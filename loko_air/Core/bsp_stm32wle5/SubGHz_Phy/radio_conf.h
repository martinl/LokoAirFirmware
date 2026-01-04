#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>

#include "bsp.h"
#include "bsp_subghz.h"
#include "mw_log_conf.h"    /* mw trace conf */
#include "radio_board_if.h" /* low layer api (bsp) */

/**
 * @brief drive value used anytime radio is NOT in TX low power mode
 * @note override the default configuration of radio_driver.c
 */
#define SMPS_DRIVE_SETTING_DEFAULT SMPS_DRV_40

/**
 * @brief drive value used anytime radio is in TX low power mode
 *        TX low power mode is the worst case because the PA sinks from SMPS
 *        while in high power mode, current is sunk directly from the battery
 * @note override the default configuration of radio_driver.c
 */
#define SMPS_DRIVE_SETTING_MAX SMPS_DRV_60

/**
 * @brief Provides the frequency of the chip running on the radio and the frequency step
 * @remark These defines are used for computing the frequency divider to set the RF frequency
 * @note override the default configuration of radio_driver.c
 */
#define XTAL_FREQ (32000000UL)

/**
 * @brief in XO mode, set internal capacitor (from 0x00 to 0x2F starting 11.2pF with 0.47pF steps)
 * @note override the default configuration of radio_driver.c
 */
#define XTAL_DEFAULT_CAP_VALUE (0x20UL)

/**
 * @brief voltage of vdd tcxo.
 * @note override the default configuration of radio_driver.c
 */
#define TCXO_CTRL_VOLTAGE TCXO_CTRL_1_7V

/**
 * @brief Radio maximum wakeup time (in ms)
 * @note override the default configuration of radio_driver.c
 */
#define RF_WAKEUP_TIME (1UL)

/**
 * @brief DCDC is enabled
 * @remark this define is only used if the DCDC is present on the board
 * @note override the default configuration of radio_driver.c
 */
#define DCDC_ENABLE (1UL)

#ifndef CRITICAL_SECTION_BEGIN
#    define CRITICAL_SECTION_BEGIN() bsp_disable_irq()
#endif /* !CRITICAL_SECTION_BEGIN */
#ifndef CRITICAL_SECTION_END
#    define CRITICAL_SECTION_END() bsp_enable_irq()
#endif /* !CRITICAL_SECTION_END */

#define RADIO_INIT                       subghz_init
#define RADIO_DELAY_MS                   bsp_delay_ms
#define RADIO_MEMSET8(dest, value, size) memset(dest, value, size)
#define RADIO_MEMCPY8(dest, src, size)   memcpy(dest, src, size)

#ifdef __cplusplus
}
#endif
