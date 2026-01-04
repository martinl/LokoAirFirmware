#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */

#define SETTINGS_LORA_DEV_EUI_SIZE (8)
#define SETTINGS_LORA_APP_EUI_SIZE (8)
#define SETTINGS_LORA_APP_KEY_SIZE (16)
#define SETTINGS_P2P_KEY_SIZE      (32)

typedef enum settings_gnss_mode_s {
    SETTINGS_GNSS_MODE_NORMAL = 0,
    SETTINGS_GNSS_MODE_FITNESS,
    SETTINGS_GNSS_MODE_AVIATION,
    SETTINGS_GNSS_MODE_BALLOON,
    SETTINGS_GNSS_MODE_STATIONARY,

    SETTINGS_GNSS_MODE_COUNT,
} settings_gnss_mode_t;

/* -------------------------------------------------------------------------- */

typedef struct PACKED {
    uint32_t id_1;
    uint32_t id_2;
    uint32_t lora_frequency_hz;
    uint32_t auto_wakeup_period_s;
    uint32_t gnss_trace_save_mult;
    int8_t tx_power;
    settings_gnss_mode_t gnss_mode;
    bool debug_output;
    bool is_lorawan_mode;
    bool is_p2p_encrypted;
    bool is_extended_packet;
    uint8_t lora_dev_eui[SETTINGS_LORA_DEV_EUI_SIZE];
    uint8_t lora_app_eui[SETTINGS_LORA_APP_EUI_SIZE];
    uint8_t lora_app_key[SETTINGS_LORA_APP_KEY_SIZE];
    uint8_t p2p_key[SETTINGS_P2P_KEY_SIZE];
    uint8_t lorawan_region_id;
} __packed settings_t;

/* -------------------------------------------------------------------------- */

typedef struct {
    void (*read)(const uint32_t offset, void *data, const uint32_t data_size);
    void (*write)(const uint32_t offset, const void *data, const uint32_t data_size);
    void (*erase)(void);
    void (*save_request_cb)(void);
    void (*default_init_cb)(settings_t *settings);
    size_t page_size;
} settings_io_t;

/* -------------------------------------------------------------------------- */

bool settings_init(const settings_io_t *io);
void settings_reload(void);
void settings_save(void);
void settings_reset(void);
void settings_print(void);

void settings_set_id_1(uint32_t id_1);
uint32_t settings_get_id_1(void);

void settings_set_id_2(uint32_t id_2);
uint32_t settings_get_id_2(void);

void settings_set_lora_frequency_hz(uint32_t lora_frequency_hz);
uint32_t settings_get_lora_frequency_hz(void);

void settings_set_auto_wakeup_period_s(uint32_t auto_wakeup_period_s);
uint32_t settings_get_auto_wakeup_period_s(void);

void settings_set_gnss_trace_save_mult(uint32_t gnss_trace_save_mult);
uint32_t settings_get_gnss_trace_save_mult(void);

void settings_set_tx_power(int8_t tx_power);
int8_t settings_get_tx_power(void);

void settings_set_gnss_mode(settings_gnss_mode_t gnss_mode);
settings_gnss_mode_t settings_get_gnss_mode(void);

void settings_set_debug_output(bool is_debug_output);
bool settings_is_debug_output(void);

void settings_set_lorawan_mode(bool is_lorawan_mode);
bool settings_get_is_lorawan_mode(void);

void settings_set_p2p_encrypted(bool is_p2p_encrypted);
bool settings_get_is_p2p_encrypted(void);

void settings_set_extended_packet(bool is_extended_packet);
bool settings_get_is_extended_packet(void);

/* LoRaWan Stuff */

void settings_set_lora_dev_eui(const uint8_t lora_dev_eui[SETTINGS_LORA_DEV_EUI_SIZE]);
void settings_get_lora_dev_eui(uint8_t lora_dev_eui[SETTINGS_LORA_DEV_EUI_SIZE]);

void settings_set_lora_app_eui(const uint8_t lora_app_eui[SETTINGS_LORA_APP_EUI_SIZE]);
void settings_get_lora_app_eui(uint8_t lora_app_eui[SETTINGS_LORA_APP_EUI_SIZE]);

void settings_set_lora_app_key(const uint8_t lora_app_key[SETTINGS_LORA_APP_KEY_SIZE]);
void settings_get_lora_app_key(uint8_t lora_app_key[SETTINGS_LORA_APP_KEY_SIZE]);

void settings_set_p2p_key(const uint8_t p2p_key[SETTINGS_P2P_KEY_SIZE]);
void settings_get_p2p_key(uint8_t p2p_key[SETTINGS_P2P_KEY_SIZE]);

void settings_set_lorawan_region_id(uint8_t lorawan_region_id);
uint8_t settings_get_lorawan_region_id(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
