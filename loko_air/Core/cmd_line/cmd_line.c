
#include <ctype.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cmd_line.h"

#include <Mac/LoRaMacInterfaces.h>
#include <bsp.h>
#include <gnss_trace.h>
#include <lora_app.h>
#include <lorawan_app/lorawan_conf.h>
#include <settings/settings.h>
#include <utils.h>
#include <version.h>

/* -------------------------------------------------------------------------- */
#define CMD_RX_BUFF_SIZE (1024)

#define _IS_CHAR_DIG(ch) ((uint32_t)(ch - '0') <= 9UL)
#define _COUNT_OF(x)     ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))

typedef struct {
    char *cmd;
    char const *(*fn)(const char *data);
    char *help;
} cmd_t;

/* -------------------------------------------------------------------------- */

static const char OK[] = "OK" CONSOLE_EOL;
static const char WRONG_ARGUMENT[] = "ERR Wrong argument" CONSOLE_EOL;
static const char UNKNOWN_COMMAND[] = "ERR Unknown command" CONSOLE_EOL;
static const char ERROR_MODE[] = "ERR wrong mode" CONSOLE_EOL;
static char _rx_data[CMD_RX_BUFF_SIZE];
static volatile size_t _rx_data_index = 0;
static char const *_cmd_help(const char *data);

static const struct {
    char *const name;
    uint8_t id;
} REGIONS[] = {
#ifdef REGION_AS923
    {.name = "AS923",  .id = LORAMAC_REGION_AS923},
#endif  // REGION_AS923
#ifdef REGION_AU915
    { .name = "AU915", .id = LORAMAC_REGION_AU915},
#endif  // REGION_AU915
#ifdef REGION_CN470
    { .name = "CN470", .id = LORAMAC_REGION_CN470},
#endif  // REGION_CN470
#ifdef REGION_CN779
    { .name = "CN779", .id = LORAMAC_REGION_CN779},
#endif  // REGION_CN779
#ifdef REGION_EU433
    { .name = "EU433", .id = LORAMAC_REGION_EU433},
#endif  // REGION_EU433
#ifdef REGION_EU868
    { .name = "EU868", .id = LORAMAC_REGION_EU868},
#endif  // REGION_EU868
#ifdef REGION_KR920
    { .name = "KR920", .id = LORAMAC_REGION_KR920},
#endif  // REGION_KR920
#ifdef REGION_IN865
    { .name = "IN865", .id = LORAMAC_REGION_IN865},
#endif  // REGION_IN865
#ifdef REGION_US915
    { .name = "US915", .id = LORAMAC_REGION_US915},
#endif  // REGION_US915
#ifdef REGION_RU864
    { .name = "RU864", .id = LORAMAC_REGION_RU864},
#endif  // REGION_RU864
};

/* -------------------------------------------------------------------------- */

extern gtrace_t *app_get_gtrace_context(void);

/* -------------------------------------------------------------------------- */

static const char *_get_next_sub_num_str(const char *str) {
    // like strtol but simpler

    while (str <= &_rx_data[sizeof(_rx_data) - 1]) {
        if (_IS_CHAR_DIG(*str) == false) {
            break;
        }

        str++;
    }

    while (str <= &_rx_data[sizeof(_rx_data) - 1]) {
        if (_IS_CHAR_DIG(*str)) {
            break;
        }

        str++;
    }

    return str;
}

/* -------------------------------------------------------------------------- */

static bool _parse_uint32_value(const char *str, uint32_t *out_dig) {
    uint64_t result = strtoull(str, NULL, 10);

    if (result <= UINT32_MAX) {
        *out_dig = (uint32_t)result;
        return true;
    }

    return false;
}

/* -------------------------------------------------------------------------- */

static bool _parse_int8_value(const char *str, int8_t *out_dig) {
    int64_t result = strtoll(str, NULL, 10);

    if ((result >= INT8_MIN) && (result <= INT8_MAX)) {
        *out_dig = (int8_t)result;
        return true;
    }

    return false;
}

/* -------------------------------------------------------------------------- */

static uint8_t _hex_to_dig(char ch) {
    if (_IS_CHAR_DIG(ch)) {
        return (uint8_t)(ch - '0');
    }

    char lower_ch = (char)tolower(ch);
#if 0
    if (lower_ch >= 'a' && lower_ch <= 'f') {
        return (uint8_t)(lower_ch - 'a' + 10);
    }

    return 0;
#else
    return (uint8_t)(lower_ch - 'a' + 10);
#endif
}

/* -------------------------------------------------------------------------- */

static void _hex_str_to_array(uint8_t *array, const char *str, size_t array_size) {
    for (size_t i = 0; i < array_size; i++) {
        array[i] = (uint8_t)(_hex_to_dig(str[i * 2]) << 4) | _hex_to_dig(str[i * 2 + 1]);
    }
}

/* -------------------------------------------------------------------------- */

static const char *_get_next_arg(const char *str, size_t size) {
    bool is_spaces_detected = false;

    for (size_t i = 0; i < size; i++) {
        if (is_spaces_detected == false && str[i] == ' ') {
            is_spaces_detected = true;
        }

        if (is_spaces_detected == true && str[i] != ' ') {
            return &str[i];
        }
    }

    return NULL;
}

/* -------------------------------------------------------------------------- */

static void _print(const char *format, ...) {

    char string[128];

    va_list args;
    va_start(args, format);
    int strlen = vsnprintf(string, LOG_MAX_MESSAGE_LENGTH, format, args);

    if (strlen >= 0) {
        cmd_output(string, (size_t)strlen);
    }
    va_end(args);
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_id_1(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);
    dig_str = _get_next_sub_num_str(dig_str);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    settings_set_id_1(dig);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_id_2(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);
    dig_str = _get_next_sub_num_str(dig_str);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    settings_set_id_2(dig);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_frequency(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    settings_set_lora_frequency_hz(dig);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_auto_wakeup_interval(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    settings_set_auto_wakeup_period_s(dig);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_reset(const char *data) {
    if ((data != NULL) && (data[6] == 'b')) {
#if CONFIG_LOKO_CPPUTEST == 0
        INTER_TARGET_MAILBOX_SEND(INTER_TARGET_MAILBOX_CMD_STAY_IN_BOOTLOADER);
#endif
    }

    bsp_system_reset();
    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_print(const char *data) {
    UNUSED(data);

    _print("Loko v%d.%d" CONSOLE_EOL, FIRMWARE_VERSION_MAJOR, FIRMWARE_VERSION_MINOR);
    _print("Flavor: " GIT_BRANCH_NAME CONSOLE_EOL);
    _print("Settings:" CONSOLE_EOL);
    _print("\t.id_1 = %ld" CONSOLE_EOL, settings_get_id_1());
    _print("\t.id_2 = %ld" CONSOLE_EOL, settings_get_id_2());
    _print("\t.lora_frequency_hz = %ld" CONSOLE_EOL, settings_get_lora_frequency_hz());
    _print("\t.tx_power = %ld" CONSOLE_EOL, settings_get_tx_power());
    _print("\t.auto_wakeup_period_s = %ld" CONSOLE_EOL, settings_get_auto_wakeup_period_s());
    _print("\t.gnss_trace_save_mult = %ld" CONSOLE_EOL, settings_get_gnss_trace_save_mult());
    _print("\t.is_lorawan_mode = %d" CONSOLE_EOL, settings_get_is_lorawan_mode());
    _print("\t.is_p2p_encrypted = %d" CONSOLE_EOL, settings_get_is_p2p_encrypted());
    _print("\t.is_debug_output = %d" CONSOLE_EOL, settings_is_debug_output());
    _print("\t.gnss_mode = %d" CONSOLE_EOL, settings_get_gnss_mode());
    _print("\t.is_extended_packet = %d" CONSOLE_EOL, settings_get_is_extended_packet());
    for (size_t i = 0; i < _COUNT_OF(REGIONS); i++) {
        if (settings_get_lorawan_region_id() == REGIONS[i].id) {
            _print("\t.lorawan_region = %s" CONSOLE_EOL, REGIONS[i].name);
            break;
        }
    }

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_erase(const char *data) {
    UNUSED(data);

    settings_reset();
    settings_save();

    bsp_flash_lorawan_nvm_erase();

    _cmd_reset(NULL);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_gtrace_print(const char *data) {
    UNUSED(data);

    gtrace_t const *gtrace = app_get_gtrace_context();
    const size_t RECORD_COUNT = gtrace_get_record_count(gtrace);

    _print("Record found %u, ID1=%" PRIu32 ", ID2=%" PRIu32 CONSOLE_EOL,
           RECORD_COUNT,
           settings_get_id_1(),
           settings_get_id_2());

    for (size_t i = 0; i < RECORD_COUNT; i++) {
        gtrace_record_t record;

        gtrace_get_record(gtrace, i, &record);

        _print("#%u %u/%u/%u %u:%u:%u %.7f, %.7f, %u, %u" CONSOLE_EOL,
               i,
               record.year,
               record.month,
               record.date,
               record.hours,
               record.minutes,
               record.seconds,
               record.latitude,
               record.longitude,
               (uint32_t)record.alt,
               (uint32_t)record.speed_mps);
    }

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_gtrace_erase(const char *data) {
    UNUSED(data);

    gtrace_erase_all(app_get_gtrace_context());

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_gtrace_period(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    settings_set_gnss_trace_save_mult(dig);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_enable_lorawan(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    if (dig > 1) {
        return WRONG_ARGUMENT;
    }

    settings_set_lorawan_mode((bool)dig);
    if (dig == 1) {
        _cmd_reset(NULL);
    }

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_enable_p2p_enc(const char *string) {
    const char *arg1 = _get_next_arg(string, 128);
    const char *dig_str = _get_next_arg(arg1, 128);

    if (dig_str == NULL) {
        return WRONG_ARGUMENT;
    }

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    if (dig > 1) {
        return WRONG_ARGUMENT;
    }

    settings_set_p2p_encrypted((bool)dig);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_dev_eui(const char *string) {
    const char *arg1_set_param_name = _get_next_arg(string, 128);
    const char *dig_str = _get_next_arg(arg1_set_param_name, 128);

    if (settings_get_is_lorawan_mode()) {
        return ERROR_MODE;
    }

    if (dig_str == NULL) {
        return WRONG_ARGUMENT;
    }

    for (size_t i = 0; i < 2 * SETTINGS_LORA_DEV_EUI_SIZE; i++) {
        if (isxdigit((uint8_t)dig_str[i]) == false) {
            return WRONG_ARGUMENT;
        }
    }

    if (dig_str[2 * SETTINGS_LORA_DEV_EUI_SIZE] != '\x00') {
        return WRONG_ARGUMENT;
    }

    uint8_t lora_dev_eui[SETTINGS_LORA_DEV_EUI_SIZE];
    _hex_str_to_array(lora_dev_eui, dig_str, SETTINGS_LORA_DEV_EUI_SIZE);
    settings_set_lora_dev_eui(lora_dev_eui);
    bsp_flash_lorawan_nvm_erase();
    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_app_eui(const char *string) {
    const char *arg1_set_param_name = _get_next_arg(string, 128);
    const char *dig_str = _get_next_arg(arg1_set_param_name, 128);

    if (settings_get_is_lorawan_mode()) {
        return ERROR_MODE;
    }

    if (dig_str == NULL) {
        return WRONG_ARGUMENT;
    }

    for (size_t i = 0; i < 2 * SETTINGS_LORA_APP_EUI_SIZE; i++) {
        if (isxdigit((uint8_t)dig_str[i]) == false) {
            return WRONG_ARGUMENT;
        }
    }

    if (dig_str[2 * SETTINGS_LORA_APP_EUI_SIZE] != '\x00') {
        return WRONG_ARGUMENT;
    }

    uint8_t lora_app_eui[SETTINGS_LORA_APP_EUI_SIZE];
    _hex_str_to_array(lora_app_eui, dig_str, SETTINGS_LORA_APP_EUI_SIZE);
    settings_set_lora_app_eui(lora_app_eui);
    bsp_flash_lorawan_nvm_erase();
    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_app_key(const char *string) {

    const char *arg1_set_param_name = _get_next_arg(string, 128);
    const char *dig_str = _get_next_arg(arg1_set_param_name, 128);

    if (settings_get_is_lorawan_mode()) {
        return ERROR_MODE;
    }

    if (dig_str == NULL) {
        return WRONG_ARGUMENT;
    }

    for (size_t i = 0; i < 2 * SETTINGS_LORA_APP_KEY_SIZE; i++) {
        if (isxdigit((uint8_t)dig_str[i]) == false) {
            return WRONG_ARGUMENT;
        }
    }

    if (dig_str[2 * SETTINGS_LORA_APP_KEY_SIZE] != '\x00') {
        return WRONG_ARGUMENT;
    }

    uint8_t lora_app_key[SETTINGS_LORA_APP_KEY_SIZE];
    _hex_str_to_array(lora_app_key, dig_str, SETTINGS_LORA_APP_KEY_SIZE);
    settings_set_lora_app_key(lora_app_key);
    bsp_flash_lorawan_nvm_erase();
    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_region(const char *string) {
    const size_t MAX_REGION_STR_LEN = 5;

    const char *arg1 = _get_next_arg(string, 128);
    arg1 = _get_next_arg(arg1, 128);

    if (arg1[0] == '?') {
        _print("Available regions:" CONSOLE_EOL);
        for (size_t i = 0; i < _COUNT_OF(REGIONS); i++) {
            _print("%s" CONSOLE_EOL, REGIONS[i].name);
        }
        return NULL;
    }

    for (size_t i = 0; i < _COUNT_OF(REGIONS); i++) {
        if (strncasecmp(arg1, REGIONS[i].name, MAX_REGION_STR_LEN) == 0) {
            settings_set_lorawan_region_id(REGIONS[i].id);
            bsp_flash_lorawan_nvm_erase();
            return NULL;
        }
    }

    return WRONG_ARGUMENT;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_p2p_key(const char *string) {

    const char *arg1_set_param_name = _get_next_arg(string, 128);
    const char *dig_str = _get_next_arg(arg1_set_param_name, 128);

    if (dig_str == NULL) {
        return WRONG_ARGUMENT;
    }

    for (size_t i = 0; i < 2 * SETTINGS_P2P_KEY_SIZE; i++) {
        if (isxdigit((uint8_t)dig_str[i]) == false) {
            return WRONG_ARGUMENT;
        }
    }

    if (dig_str[2 * SETTINGS_P2P_KEY_SIZE] != '\x00') {
        return WRONG_ARGUMENT;
    }

    uint8_t p2p_key[SETTINGS_P2P_KEY_SIZE];
    _hex_str_to_array(p2p_key, dig_str, SETTINGS_P2P_KEY_SIZE);
    settings_set_p2p_key(p2p_key);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_debug_output(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    settings_set_debug_output(dig == 0 ? false : true);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_extended_packet(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    settings_set_extended_packet(dig == 0 ? false : true);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_gnss_mode(const char *string) {
    const char *dig_str = _get_next_sub_num_str(string);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    uint32_t dig = 0;
    if (_parse_uint32_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    if (dig >= SETTINGS_GNSS_MODE_COUNT) {
        return WRONG_ARGUMENT;
    }

    settings_set_gnss_mode((settings_gnss_mode_t)dig);

    return NULL;
}

/* -------------------------------------------------------------------------- */

static char const *_cmd_set_tx_power(const char *string) {

    const char *dig_str = _get_next_sub_num_str(string);

    if (!_IS_CHAR_DIG(*dig_str)) {
        return WRONG_ARGUMENT;
    }

    /* move to space or minus to consider minus */
    dig_str--;

    int8_t dig = 0;
    if (_parse_int8_value(dig_str, &dig) == false) {
        return WRONG_ARGUMENT;
    }

    settings_set_tx_power(dig);

    return NULL;
}

/* -------------------------------------------------------------------------- */
// clang-format off
static cmd_t _cmd_list[] = {
    { "help",                _cmd_help,                     NULL                                                                                   },
    { "reset",               _cmd_reset,                    "Reset MCU"                                                                            },
    { "info",                _cmd_print,                    "Print settings"                                                                       },
    { "erase",               _cmd_erase,                    "Erase settings"                                                                       },
    { "set id1",             _cmd_set_id_1,                 "Ex:set id1 123"                                                                       },
    { "set id2",             _cmd_set_id_2,                 "Ex:set id2 456"                                                                       },
    { "set freq",            _cmd_set_frequency,            "Frequency in HZ, Ex:set freq 866000000"                                               },
    { "set interval",        _cmd_set_auto_wakeup_interval, "Auto wake-up interval in seconds, Ex:set interval 60"                                 },
    { "gtrace print",        _cmd_gtrace_print,             "Show all gnss traces"                                                                 },
    { "gtrace erase",        _cmd_gtrace_erase,             "Erase all gnss records"                                                               },
    { "gtrace period",       _cmd_gtrace_period,            "Set wakeup count before write GNSS trace. Ex:gtrace period 10"                        },
    { "enable lorawan mode", _cmd_enable_lorawan,           "Enable or disable LoRaWAN mode. Ex:enable lorawan mode 1"                             },
    { "set dev-eui",         _cmd_set_dev_eui,              "Set end-device IEEE EUI. Ex:set dev-eui 0123456789ABCDEF"                             },
    { "set app-eui",         _cmd_set_app_eui,              "Set App/Join server IEEE EUI. Ex:set app-eui 0123456789ABCDEF"                        },
    { "set app-key",         _cmd_set_app_key,              "Set Application root key LoRaWAN key. Ex:set app-key 0123456789ABCDEF0123456789ABCDEF"},
    { "set region",          _cmd_set_region,               "Set LoRaWAN Active Region, use \"set region ?\" to print avalble regions"             },
    { "p2p encryption",      _cmd_enable_p2p_enc,           "Enable or disable p2p encryption. Ex:p2p encryption 1"                                },
    { "set p2p-key",         _cmd_set_p2p_key,              "Set Point to Point 32bit encryption key. Ex:set p2p-key 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF"},
    { "debug",               _cmd_set_debug_output,         "Enable debug output, Ex:debug 1"                                                      },
    { "extended",            _cmd_set_extended_packet,      "Enable extended packet Ex:extended 1"                                                 },
    { "set gnss mode",       _cmd_set_gnss_mode,            "Set navigation mode, allowed: 0-normal, 1-fitness, 2-aviation, 3-balloon, 4-stationary. Ex: set gnss mode 1"},
    { "set tx",              _cmd_set_tx_power,             "Set lora TX power. Ex: set tx 10"},
};
// clang-format on

/* -------------------------------------------------------------------------- */

static char const *_cmd_help(const char *data) {
    UNUSED(data);

    _print("Command list:" CONSOLE_EOL);

    for (size_t i = 0; i < _COUNT_OF(_cmd_list); i++) {
        _print("    ");
        _print(_cmd_list[i].cmd);

        if (_cmd_list[i].help) {
            _print(" - ");
            _print(_cmd_list[i].help);
        }

        _print("" CONSOLE_EOL);
    }

    return NULL;
}

/* -------------------------------------------------------------------------- */

void cmd_line_receive(uint8_t byte) {

    /* stm32 bootloader entry workaround */
    static uint8_t previous_byte = 0;
    if ((byte == 0x7F) && (previous_byte == 0x7F)) {
        _rx_data_index = 0;
        uint8_t ack = 0x79;
        bsp_uart_debug_write(&ack, sizeof(ack));
        bsp_delay_ms(20);
        _cmd_reset("reset bootloader");
        return;
    }
    previous_byte = byte;

    if (byte == '\n' || byte == '\r') {
        char const *ret = UNKNOWN_COMMAND;

        if (_rx_data_index > 0) {
            for (size_t i = 0; i < _COUNT_OF(_cmd_list); i++) {
                if (memcmp(_cmd_list[i].cmd, _rx_data, strlen(_cmd_list[i].cmd)) == 0) {
                    ret = _cmd_list[i].fn(_rx_data);

                    if (ret == NULL /*&& _is_echo_en*/) {
                        _print(OK);
                    }

                    break;
                }
            }

            _rx_data_index = 0;
            memset(_rx_data, 0, sizeof(_rx_data));
        }

        if (ret != NULL) {
            _print(ret);
        }

        return;
    }

    if (_rx_data_index >= sizeof(_rx_data)) {
        _rx_data_index = 0;
        memset(_rx_data, 0, sizeof(_rx_data));

        return;
    }

    if (byte == 0x08 /*Backspace*/) {
        if (_rx_data_index > 0) {
            _rx_data[_rx_data_index] = 0;
            _rx_data_index--;
        }

        return;
    }

    _rx_data[_rx_data_index] = (char)tolower(byte);
    _rx_data_index++;
}

/* -------------------------------------------------------------------------- */

void cmd_output(char const *string, size_t len) {
    bsp_uart_debug_write((const uint8_t *)string, len);
}

/* -------------------------------------------------------------------------- */
