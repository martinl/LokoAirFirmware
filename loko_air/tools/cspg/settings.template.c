#include "settings.h"
#include <assert.h>
#include <string.h>

#define AUTO_SAVE_DATA     (1)
#define SETTINGS_SIGNATURE (0xA55A)
#define LOG_PREFIX         "SETTINGS: "
#define _LOG(...)          LOG(LOG_MASK_DEBUG, LOG_COLOR(LOG_COLOR_PURPLE) LOG_PREFIX __VA_ARGS__)
#define _LOG_ARRAY(...)    LOG_DEBUG_ARRAY(LOG_PREFIX __VA_ARGS__)

/*__SETTINGS_C_DEFAUL_SETTINGS__*/

#define UNALIGNED_BYTES ((sizeof(settings_t) + 4) % 8)
typedef struct {
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
    /*__SETTINGS_C_LOG_INFO_PRINTS__*/
}

/* -------------------------------------------------------------------------- */

void settings_init(const settings_io_t *io) {
    if (io == NULL) {
        LOG_ERROR("Invalid pointer on settings interface");
        return;
    }

    if (io->erase == NULL) {
        LOG_ERROR("Invalid pointer on settings erase interface");
        return;
    }

    if (io->page_size == 0) {
        LOG_ERROR("Invalid page_size");
        return;
    }

    if (io->read == NULL) {
        LOG_ERROR("Invalid pointer on settings read interface");
        return;
    }

    if (io->write == NULL) {
        LOG_ERROR("Invalid pointer on settings write interface");
        return;
    }

    _io = io;
    settings_reload();
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
                // break;
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
        _storage.index     = 0;
        _storage.checksum  = _checksum((const uint8_t *)&_storage, SETTINGS_CHECKSUM_BLOCK_SIZE);
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
/*__SETTINGS_C_GETTER_SETTER_FUNCTIONS__*/
