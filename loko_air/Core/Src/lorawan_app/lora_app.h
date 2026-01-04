#pragma once

#ifdef __cplusplus
extern "C" {
#endif

/* LoraWAN application configuration (Mw is configured by lorawan_conf.h) */
// #define ACTIVE_REGION LORAMAC_REGION_EU868 DO NOT USED< USED Settings Instead

/*!
 * LoRaWAN User application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_USER_APP_PORT 2

/*!
 * LoRaWAN Switch class application port
 * @note do not use 224. It is reserved for certification
 */
#define LORAWAN_SWITCH_CLASS_PORT 3

/*!
 * LoRaWAN default class
 */
#define LORAWAN_DEFAULT_CLASS CLASS_A

/*!
 * LoRaWAN default confirm state
 */
#define LORAWAN_DEFAULT_CONFIRMED_MSG_STATE LORAMAC_HANDLER_UNCONFIRMED_MSG

/*!
 * LoRaWAN Adaptive Data Rate
 * @note Please note that when ADR is enabled the end-device should be static
 */
#define LORAWAN_ADR_STATE LORAMAC_HANDLER_ADR_ON

/*!
 * LoRaWAN Default data Rate Data Rate
 * @note Please note that LORAWAN_DEFAULT_DATA_RATE is used only when LORAWAN_ADR_STATE is disabled
 */
#define LORAWAN_DEFAULT_DATA_RATE DR_0

/*!
 * LoRaWAN default activation type
 */
#define LORAWAN_DEFAULT_ACTIVATION_TYPE ACTIVATION_TYPE_OTAA

/*!
 * LoRaWAN force rejoin even if the NVM context is restored
 * @note useful only when context management is enabled by CONTEXT_MANAGEMENT_ENABLED
 */
#define LORAWAN_FORCE_REJOIN_AT_BOOT true

/*!
 * User application data buffer size
 */
#define LORAWAN_APP_DATA_BUFFER_MAX_SIZE 242

/*!
 * Default Unicast ping slots periodicity
 *
 * \remark periodicity is equal to 2^LORAWAN_DEFAULT_PING_SLOT_PERIODICITY seconds
 *         example: 2^4 = 16 seconds. The end-device will open an Rx slot every 16 seconds.
 */
#define LORAWAN_DEFAULT_PING_SLOT_PERIODICITY 4

/*!
 * Default response timeout for class b and class c confirmed
 * downlink frames in milli seconds.
 *
 * The value shall not be smaller than RETRANSMIT_TIMEOUT plus
 * the maximum time on air.
 */
#define LORAWAN_DEFAULT_CLASS_B_C_RESP_TIMEOUT 8000

/**
 * @brief  Init Lora Application
 */
void lorawan_init(void);
void lorawan_process(void);
bool lorawan_deinit(void);
bool lorawan_is_joined(void);
bool lorawan_is_tx_complete(void);
void lorawan_send(void const *data, uint8_t size);

#ifdef __cplusplus
}
#endif
