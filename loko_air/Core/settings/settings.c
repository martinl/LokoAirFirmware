#include "settings.h"
#include <assert.h>
#include <string.h>

#define AUTO_SAVE_DATA     (1)
#define SETTINGS_SIGNATURE (0xA55A)
#define LOG_PREFIX         "SETTINGS: "
#define _LOG(...)          LOG(LOG_MASK_DEBUG, LOG_COLOR(LOG_COLOR_PURPLE) LOG_PREFIX __VA_ARGS__)
#define _LOG_ARRAY(...)    LOG_DEBUG_ARRAY(LOG_PREFIX __VA_ARGS__)

static const settings_t _default = {
    .id_1 = 0,
    .id_2 = 0,
    .lora_frequency_hz = 868000000,
    .auto_wakeup_period_s = 60,
    .gnss_trace_save_mult = 5,
    .tx_power = 8,
    .gnss_mode = SETTINGS_GNSS_MODE_NORMAL,
    .is_lorawan_mode = false,
    .debug_output = false,
    .is_extended_packet = false,
    .lora_dev_eui = { 0x00 },
    .lora_app_eui = { 0x00 },
    .lora_app_key = { 0x00 },
    .lorawan_region_id = 5, /* LORAMAC_REGION_EU868, */
};

#define UNALIGNED_BYTES ((sizeof(settings_t) + 4) % 8)
typedef struct PACKED {
    settings_t settings;
    uint16_t signature;
    uint8_t index;
    uint8_t checksum;
    uint8_t __align8_dummy[UNALIGNED_BYTES > 0 ? 8 - UNALIGNED_BYTES : 0];
} __packed settings_packet_t;

#define SETTINGS_CHECKSUM_BLOCK_SIZE ((size_t) & ((settings_packet_t *)NULL)->checksum)

static settings_packet_t _storage;
static const settings_io_t *_io = NULL;

#if defined(static_assert)
static_assert((sizeof(settings_packet_t) % 8) == 0);
#endif
/* -------------------------------------------------------------------------- */

static uint8_t _checksum(uint8_t const *ptr, size_t sz) {
    uint8_t chk = 0;

    while (sz-- != 0) {
        chk -= *ptr++;
    }

    return chk;
}

/* -------------------------------------------------------------------------- */

static void _save_request(void) {
    if (_io->save_request_cb) {
        _io->save_request_cb();
    }
}

/* -------------------------------------------------------------------------- */

void settings_print(void) {
    _LOG("Settings packet size %d, packet size %d", sizeof(_storage.settings), sizeof(_storage));

    _LOG("\t.id_1 = %ld", _storage.settings.id_1);
    _LOG("\t.id_2 = %ld", _storage.settings.id_2);
    _LOG("\t.lora_frequency_hz = %ld", _storage.settings.lora_frequency_hz);
    _LOG("\t.auto_wakeup_period_s = %ld", _storage.settings.auto_wakeup_period_s);
    _LOG("\t.gnss_trace_save_mult = %ld", _storage.settings.gnss_trace_save_mult);
    _LOG("\t.tx_power = %ld", (int32_t)_storage.settings.tx_power);
    _LOG("\t.gnss_mode = %ld", _storage.settings.gnss_mode);
    _LOG("\t.is_lorawan_mode = %d", _storage.settings.is_lorawan_mode);
    _LOG("\t.debug_output = %ld", _storage.settings.debug_output);
    _LOG("\t.is_p2p_encrypted = %d", _storage.settings.is_p2p_encrypted);
    _LOG("\t.is_extended_packet = %d", _storage.settings.is_extended_packet);
    _LOG("\t.lorawan_region_id = %d", _storage.settings.lorawan_region_id);
    LOG_DEBUG_ARRAY("\t.lora_dev_eui", _storage.settings.lora_dev_eui, sizeof(_storage.settings.lora_dev_eui));
    LOG_DEBUG_ARRAY("\t.lora_app_eui", _storage.settings.lora_app_eui, sizeof(_storage.settings.lora_app_eui));
    LOG_DEBUG_ARRAY("\t.lora_app_key", _storage.settings.lora_app_key, sizeof(_storage.settings.lora_app_key));
    LOG_DEBUG_ARRAY("\t.p2p_key", _storage.settings.p2p_key, sizeof(_storage.settings.p2p_key));
}

/* -------------------------------------------------------------------------- */

void settings_reset(void) {
    memcpy(&_storage.settings, &_default, sizeof(settings_t));

    if (_io->default_init_cb) {
        _io->default_init_cb(&_storage.settings);
    }

    _storage.signature = SETTINGS_SIGNATURE;
    _storage.index = 0;
    _storage.checksum = _checksum((const uint8_t *)&_storage, SETTINGS_CHECKSUM_BLOCK_SIZE);
    _io->erase();
    _io->write(0, (uint8_t *)&_storage, sizeof(_storage));

    LOG_WARNING(LOG_PREFIX "\r\n\r\n\r\n\t\t!!!Settings reset to default!!!\r\n\r\n");
}

/* -------------------------------------------------------------------------- */

bool settings_init(const settings_io_t *io) {
    if (io == NULL) {
        LOG_ERROR("Invalid pointer on settings interface");
        return false;
    }

    if (io->erase == NULL) {
        LOG_ERROR("Invalid pointer on settings erase interface");
        return false;
    }

    if (io->page_size == 0) {
        LOG_ERROR("Invalid page_size");
        return false;
    }

    if (io->read == NULL) {
        LOG_ERROR("Invalid pointer on settings read interface");
        return false;
    }

    if (io->write == NULL) {
        LOG_ERROR("Invalid pointer on settings write interface");
        return false;
    }

    _io = io;
    settings_reload();

    return true;
}

/* -------------------------------------------------------------------------- */

void settings_reload(void) {
    settings_packet_t storage_dump;

    const size_t max_records = _io->page_size / sizeof(settings_packet_t);

    size_t last_valid_record = max_records;

    _LOG("Settings records per page %d", last_valid_record);

    for (size_t i = 0; i < max_records; i++) {
        _io->read(sizeof(settings_packet_t) * i, &storage_dump, sizeof(settings_packet_t));
        //_LOG("Record #%d, Signature 0x%04x", i, storage_ptr[i].signature);

        if (storage_dump.signature == SETTINGS_SIGNATURE) {
            uint8_t checksum = _checksum((const uint8_t *)&storage_dump, SETTINGS_CHECKSUM_BLOCK_SIZE);
            //_LOG("Checksum expected 0x%02X, calculated 0x%02x", storage_ptr[i].checksum, checksum);

            if (storage_dump.checksum == checksum) {
                last_valid_record = i;
            } else {
                LOG_WARNING(LOG_PREFIX "Found bad checksum settings record %d", i);
            }
        }
    }

    if (last_valid_record < max_records) {
        _io->read(sizeof(settings_packet_t) * last_valid_record, &_storage, sizeof(settings_packet_t));
        _LOG("load successful, index %d", last_valid_record);
    } else {
        memcpy(&_storage.settings, &_default, sizeof(settings_t));

        if (_io->default_init_cb) {
            _io->default_init_cb(&_storage.settings);
        }

        _storage.signature = SETTINGS_SIGNATURE;
        _storage.index = 0;
        _storage.checksum = _checksum((const uint8_t *)&_storage, SETTINGS_CHECKSUM_BLOCK_SIZE);
        _io->erase();
        _io->write(0, (uint8_t *)&_storage, sizeof(_storage));
        LOG_WARNING(LOG_PREFIX "\r\n\r\n\r\n\t\t!!!Settings not found or structure changed, using default!!!\r\n\r\n");
    }

    settings_print();
}

/* -------------------------------------------------------------------------- */

void settings_save(void) {
    settings_packet_t storage_last;
    _io->read(sizeof(settings_packet_t) * _storage.index, &storage_last, sizeof(settings_packet_t));

    if (memcmp(&_storage.settings, &storage_last.settings, sizeof(_storage.settings)) != 0) {
        _storage.index++;
        const size_t max_records = _io->page_size / sizeof(settings_packet_t);

        if (_storage.index >= max_records) {
            _storage.index = 0;
            _LOG("Erase settings");
            _io->erase();
        }

        _storage.checksum = _checksum((const uint8_t *)&_storage, SETTINGS_CHECKSUM_BLOCK_SIZE);

        bool is_matched = false;

        size_t offset = sizeof(_storage) * _storage.index;
        LOG_DEBUG("Write offset 0x%X, size 0x%x", offset, sizeof(_storage));
        _LOG("save, id %d", _storage.index);

        _io->write(offset, (uint8_t *)&_storage, sizeof(_storage));
        _io->read(sizeof(settings_packet_t) * _storage.index, &storage_last, sizeof(settings_packet_t));
        is_matched = memcmp(&_storage, &storage_last, sizeof(_storage)) == 0;

        if (!is_matched) {
            LOG_ERROR("Settings Compare failed after write");
        }
    } else {
        _LOG("no need save");
    }
}

/* -------------------------------------------------------------------------- */

uint32_t settings_get_id_1(void) {
    return _storage.settings.id_1;
}

/* -------------------------------------------------------------------------- */

void settings_set_id_1(uint32_t id_1) {
    _LOG("Set .id_1 = %ld(0x%lX)", id_1, id_1);
    _storage.settings.id_1 = id_1;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

uint32_t settings_get_id_2(void) {
    return _storage.settings.id_2;
}

/* -------------------------------------------------------------------------- */

void settings_set_id_2(uint32_t id_2) {
    _LOG("Set .id_2 = %ld(0x%lX)", id_2, id_2);
    _storage.settings.id_2 = id_2;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

uint32_t settings_get_lora_frequency_hz(void) {
    return _storage.settings.lora_frequency_hz;
}

/* -------------------------------------------------------------------------- */

void settings_set_lora_frequency_hz(uint32_t lora_frequency_hz) {
    _LOG("Set .lora_frequency_hz = %ld(0x%lX)", lora_frequency_hz, lora_frequency_hz);
    _storage.settings.lora_frequency_hz = lora_frequency_hz;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

uint32_t settings_get_auto_wakeup_period_s(void) {
    return _storage.settings.auto_wakeup_period_s;
}

/* -------------------------------------------------------------------------- */

void settings_set_auto_wakeup_period_s(uint32_t auto_wakeup_period_s) {
    _LOG("Set .auto_wakeup_period_s = %ld(0x%lX)", auto_wakeup_period_s, auto_wakeup_period_s);
    _storage.settings.auto_wakeup_period_s = auto_wakeup_period_s;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

uint32_t settings_get_gnss_trace_save_mult(void) {
    return _storage.settings.gnss_trace_save_mult;
}

/* -------------------------------------------------------------------------- */

void settings_set_gnss_trace_save_mult(uint32_t gnss_trace_save_mult) {
    _LOG("Set .gnss_trace_save_mult = %ld(0x%lX)", gnss_trace_save_mult, gnss_trace_save_mult);
    _storage.settings.gnss_trace_save_mult = gnss_trace_save_mult;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

bool settings_get_is_lorawan_mode(void) {
    return _storage.settings.is_lorawan_mode;
}

/* -------------------------------------------------------------------------- */

void settings_set_lorawan_mode(bool is_lorawan_mode) {
    _LOG("Set .is_lorawan_mode = %d", is_lorawan_mode);
    _storage.settings.is_lorawan_mode = is_lorawan_mode;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}
/* -------------------------------------------------------------------------- */

bool settings_get_is_p2p_encrypted(void) {
    return _storage.settings.is_p2p_encrypted;
}

/* -------------------------------------------------------------------------- */

void settings_set_p2p_encrypted(bool is_p2p_encrypted) {
    _LOG("Set .is_p2p_encrypted = %d", is_p2p_encrypted);
    _storage.settings.is_p2p_encrypted = is_p2p_encrypted;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

void settings_get_lora_dev_eui(uint8_t lora_dev_eui[SETTINGS_LORA_DEV_EUI_SIZE]) {
    memcpy(lora_dev_eui, &_storage.settings.lora_dev_eui, sizeof(_storage.settings.lora_dev_eui));
}

/* -------------------------------------------------------------------------- */

void settings_set_lora_dev_eui(const uint8_t lora_dev_eui[SETTINGS_LORA_DEV_EUI_SIZE]) {

    LOG_DEBUG_ARRAY_BLUE("Set .lora_dev_eui", lora_dev_eui, SETTINGS_LORA_DEV_EUI_SIZE);
    memcpy(&_storage.settings.lora_dev_eui, lora_dev_eui, sizeof(_storage.settings.lora_dev_eui));

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

void settings_get_lora_app_eui(uint8_t lora_app_eui[SETTINGS_LORA_APP_EUI_SIZE]) {
    memcpy(lora_app_eui, &_storage.settings.lora_app_eui, sizeof(_storage.settings.lora_app_eui));
}

/* -------------------------------------------------------------------------- */

void settings_set_lora_app_eui(const uint8_t lora_app_eui[SETTINGS_LORA_APP_EUI_SIZE]) {

    LOG_DEBUG_ARRAY_BLUE("Set .lora_app_eui", lora_app_eui, SETTINGS_LORA_APP_EUI_SIZE);
    memcpy(&_storage.settings.lora_app_eui, lora_app_eui, sizeof(_storage.settings.lora_app_eui));

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

void settings_get_lora_app_key(uint8_t lora_app_key[SETTINGS_LORA_APP_KEY_SIZE]) {
    memcpy(lora_app_key, &_storage.settings.lora_app_key, sizeof(_storage.settings.lora_app_key));
}

/* -------------------------------------------------------------------------- */

void settings_set_lora_app_key(const uint8_t lora_app_key[SETTINGS_LORA_APP_KEY_SIZE]) {

    LOG_DEBUG_ARRAY_BLUE("Set .lora_app_key", lora_app_key, SETTINGS_LORA_APP_KEY_SIZE);
    memcpy(&_storage.settings.lora_app_key, lora_app_key, sizeof(_storage.settings.lora_app_key));

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

void settings_get_p2p_key(uint8_t p2p_key[SETTINGS_P2P_KEY_SIZE]) {
    memcpy(p2p_key, &_storage.settings.p2p_key, sizeof(_storage.settings.p2p_key));
}

/* -------------------------------------------------------------------------- */

void settings_set_p2p_key(const uint8_t p2p_key[SETTINGS_P2P_KEY_SIZE]) {

    LOG_DEBUG_ARRAY_BLUE("Set .lora_app_key", p2p_key, SETTINGS_P2P_KEY_SIZE);
    memcpy(&_storage.settings.p2p_key, p2p_key, sizeof(_storage.settings.p2p_key));

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

bool settings_is_debug_output(void) {
    return _storage.settings.debug_output;
}

/* -------------------------------------------------------------------------- */

void settings_set_debug_output(bool is_debug_output) {

    _LOG("Set .debug_output = %ld", is_debug_output);
    _storage.settings.debug_output = is_debug_output;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

void settings_set_extended_packet(bool is_extended_packet) {
    _LOG("Set .is_extended_packet = %d", is_extended_packet);
    _storage.settings.is_extended_packet = is_extended_packet;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

bool settings_get_is_extended_packet(void) {
    return _storage.settings.is_extended_packet;
}

/* -------------------------------------------------------------------------- */

void settings_set_gnss_mode(settings_gnss_mode_t gnss_mode) {
    _LOG("Set .gnss_mode = %d", gnss_mode);
    _storage.settings.gnss_mode = gnss_mode;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

settings_gnss_mode_t settings_get_gnss_mode(void) {
    return _storage.settings.gnss_mode;
}

/* -------------------------------------------------------------------------- */

void settings_set_tx_power(int8_t tx_power) {
    _LOG("Set .tx_power = %d", tx_power);
    _storage.settings.tx_power = tx_power;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

int8_t settings_get_tx_power(void) {
    return _storage.settings.tx_power;
}

/* -------------------------------------------------------------------------- */

void settings_set_lorawan_region_id(uint8_t lorawan_region_id) {
    _LOG("Set .lorawan_region_id = %d", lorawan_region_id);
    _storage.settings.lorawan_region_id = lorawan_region_id;

#if AUTO_SAVE_DATA == 1
    _save_request();
#endif
}

/* -------------------------------------------------------------------------- */

uint8_t settings_get_lorawan_region_id(void) {
    return _storage.settings.lorawan_region_id;
}

/* -------------------------------------------------------------------------- */
