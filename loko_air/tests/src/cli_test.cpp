#include "CppUTest/TestHarness.h"

#include "cmd_line.h"
#include <bsp.h>
#include <gnss_trace.h>
#include <lorawan_app/lorawan_conf.h>
#include <settings.h>
#include <spy/settings_io.hpp>

extern "C" {
extern gtrace_t *app_get_gtrace_context(void);
}

TEST_GROUP(cli_test) {
    char rx_buffer[1024 * 10];
    size_t buffer_size = 0;

    void setup() override {
        settings_setup_io();
        settings_init(settings_get_io());
        cmd_line_receive('\n');
    }

    void teardown() override {
        //
    }

    void cli_send(const char cmd[]) {
        bsp_uart_debug_drop_buffer();

        while (*cmd != 0) {
            cmd_line_receive((uint8_t)(*cmd));
            cmd++;
        }
        memset(rx_buffer, 0, sizeof(rx_buffer));
        buffer_size = bsp_uart_debug_get_buffer(rx_buffer, sizeof(rx_buffer));
    }
};

TEST(cli_test, wrong_cmd) {
    cli_send("\r");
    STRCMP_EQUAL("ERR Unknown command" CONSOLE_EOL, rx_buffer);

    cli_send("s\r");
    STRCMP_EQUAL("ERR Unknown command" CONSOLE_EOL, rx_buffer);

    cli_send("set\r");
    STRCMP_EQUAL("ERR Unknown command" CONSOLE_EOL, rx_buffer);

    cli_send("set \r");
    STRCMP_EQUAL("ERR Unknown command" CONSOLE_EOL, rx_buffer);

    cli_send(".set \r");
    STRCMP_EQUAL("ERR Unknown command" CONSOLE_EOL, rx_buffer);
}

TEST(cli_test, wrong_int_arg) {
    cli_send("set id1\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set id1 \r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set id1 chacha\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set id1 1\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
}

TEST(cli_test, check_long_cmd) {
    bsp_uart_debug_drop_buffer();

    for (size_t i = 0; i < 2 * 1024; i++) {
        cmd_line_receive('X');
    }
    CHECK_EQUAL(0, bsp_uart_debug_get_buffer(rx_buffer, sizeof(rx_buffer)));

    cli_send("\r");

    cli_send("set id12");
    for (size_t i = 0; i < 1024 - 8; i++) {
        cmd_line_receive('1');
    }

    cli_send("\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
}

TEST(cli_test, check_backspace) {
    cli_send("help\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    cli_send("h\x08help\x08p\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    cli_send("h\x08\x08\x08help\x08p\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
}

TEST(cli_test, common_command_ok_test) {
    cli_send("help\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    cli_send("reset\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    cli_send("reset bootloader\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    cli_send("info\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
}

TEST(cli_test, command_set_id_x) {
    cli_send("set id1 1\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_id_1());

    cli_send("set id1 4294967295\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(UINT32_MAX, settings_get_id_1());

    cli_send("set id1 123\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(123, settings_get_id_1());

    cli_send("set id1 12345\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345, settings_get_id_1());

    cli_send("set id1 12345678\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_id_1());

    cli_send("set id1 12345678000123\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_id_1());

    cli_send("set id1 ccc\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_id_1());

    ///

    cli_send("set id2 1\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_id_2());

    cli_send("set id2 4294967295\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(UINT32_MAX, settings_get_id_2());

    cli_send("set id2 123\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(123, settings_get_id_2());

    cli_send("set id2 12345\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345, settings_get_id_2());

    cli_send("set id2 12345678\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_id_2());

    cli_send("set id2 12345678000123\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_id_2());

    cli_send("set id2 ccc\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_id_2());
}

TEST(cli_test, command_set_freq) {
    cli_send("set freq 866000000\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(866000000, settings_get_lora_frequency_hz());

    cli_send("set freq 000000\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(000000, settings_get_lora_frequency_hz());

    cli_send("set freq 1866000000\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(1866000000, settings_get_lora_frequency_hz());

    cli_send("set freq 1111111866000000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1866000000, settings_get_lora_frequency_hz());

    cli_send("set freq\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1866000000, settings_get_lora_frequency_hz());

    cli_send("set freq vvvv\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1866000000, settings_get_lora_frequency_hz());
}

TEST(cli_test, command_set_interval) {
    cli_send("set interval 55\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(55, settings_get_auto_wakeup_period_s());

    cli_send("set interval 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_get_auto_wakeup_period_s());

    cli_send("set interval 12345678\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(12345678, settings_get_auto_wakeup_period_s());

    cli_send("set interval 12345678000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_auto_wakeup_period_s());

    cli_send("set interval\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_auto_wakeup_period_s());

    cli_send("set interval hh\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(12345678, settings_get_auto_wakeup_period_s());
}

TEST(cli_test, command_set_trace_period) {
    cli_send("gtrace period 10\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(10, settings_get_gnss_trace_save_mult());

    cli_send("gtrace period 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_get_gnss_trace_save_mult());

    cli_send("gtrace period 123456789\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(123456789, settings_get_gnss_trace_save_mult());

    cli_send("gtrace period\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(123456789, settings_get_gnss_trace_save_mult());

    cli_send("gtrace period 1234567890123\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(123456789, settings_get_gnss_trace_save_mult());
}

TEST(cli_test, command_gtrace_erase) {
    cli_send("gtrace erase\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
}

TEST(cli_test, command_set_id_) {
    cli_send("gtrace print\r");
    STRCMP_EQUAL("Record found 0, ID1=0, ID2=0" CONSOLE_EOL "OK" CONSOLE_EOL, rx_buffer);

    gtrace_t *context = app_get_gtrace_context();
    gtrace_init(context);
    gtrace_erase_all(context);
    gtrace_record_t record;
    record.latitude = 321.123456789;
    record.longitude = 123.123456789;
    record.date = 13;
    record.month = 4;
    record.year = 23;
    record.hours = 1;
    record.minutes = 2;
    record.seconds = 59;
    record.alt = 321;
    record.speed_mps = 123;
    gtrace_add(context, &record);

    cli_send("gtrace print\r");
    STRCMP_EQUAL("Record found 1, ID1=0, ID2=0" CONSOLE_EOL
                 "#0 23/4/13 1:2:59 321.1234568, 123.1234568, 321, 123" CONSOLE_EOL "OK" CONSOLE_EOL,
                 rx_buffer);
}

TEST(cli_test, command_enable_lorawan) {
    cli_send("enable lorawan mode 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_get_is_lorawan_mode());

    cli_send("enable lorawan mode 1\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(1, settings_get_is_lorawan_mode());

    cli_send("enable lorawan mode\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_is_lorawan_mode());

    cli_send("enable lorawan mode 1234567890123\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_is_lorawan_mode());

    cli_send("enable lorawan mode 2\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_is_lorawan_mode());
}

TEST(cli_test, command_set_dev_eui) {
    cli_send("set dev-eui\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set dev-eui chars\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set dev-eui 0123456789ABCDEF000000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set dev-eui 0123456789ABCDEF\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    uint8_t experted_lora_dev_eui[SETTINGS_LORA_DEV_EUI_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    uint8_t lora_dev_eui[SETTINGS_LORA_DEV_EUI_SIZE];
    settings_get_lora_dev_eui(lora_dev_eui);
    CHECK_EQUAL(0, memcmp(lora_dev_eui, experted_lora_dev_eui, SETTINGS_LORA_DEV_EUI_SIZE));

    cli_send("set dev-eui 0000000000000000\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t experted_lora_dev_eui2[SETTINGS_LORA_DEV_EUI_SIZE] = { 0x00 };
    settings_get_lora_dev_eui(lora_dev_eui);
    CHECK_EQUAL(0, memcmp(lora_dev_eui, experted_lora_dev_eui2, SETTINGS_LORA_DEV_EUI_SIZE));

    cli_send("set dev-eui 0FffFFffFFffFFff\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t experted_lora_dev_eui3[SETTINGS_LORA_DEV_EUI_SIZE] = { 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    settings_get_lora_dev_eui(lora_dev_eui);
    CHECK_EQUAL(0, memcmp(lora_dev_eui, experted_lora_dev_eui3, SETTINGS_LORA_DEV_EUI_SIZE));

    cli_send("set dev-eui fFffFFffFFffFFff\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t experted_lora_dev_eui4[SETTINGS_LORA_DEV_EUI_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    settings_get_lora_dev_eui(lora_dev_eui);
    CHECK_EQUAL(0, memcmp(lora_dev_eui, experted_lora_dev_eui4, SETTINGS_LORA_DEV_EUI_SIZE));

    settings_set_lorawan_mode(true);
    cli_send("set dev-eui 0FffFFffFFffFFff\r");
    STRCMP_EQUAL("ERR wrong mode" CONSOLE_EOL, rx_buffer);
}

TEST(cli_test, command_set_app_eui) {
    cli_send("set app-eui\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set app-eui chars\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set app-eui 0123456789ABCDEF000000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set app-eui 0123456789ABCDEF\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    uint8_t experted_lora_dev_eui[SETTINGS_LORA_APP_EUI_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    uint8_t lora_app_eui[SETTINGS_LORA_APP_EUI_SIZE];
    settings_get_lora_app_eui(lora_app_eui);
    CHECK_EQUAL(0, memcmp(lora_app_eui, experted_lora_dev_eui, SETTINGS_LORA_APP_EUI_SIZE));

    cli_send("set app-eui 0000000000000000\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t expected_lora_app_eui2[SETTINGS_LORA_APP_EUI_SIZE] = { 0x00 };
    settings_get_lora_app_eui(lora_app_eui);
    CHECK_EQUAL(0, memcmp(lora_app_eui, expected_lora_app_eui2, SETTINGS_LORA_APP_EUI_SIZE));

    cli_send("set app-eui 0FffFFffFFffFFff\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t expected_lora_app_eui3[SETTINGS_LORA_APP_EUI_SIZE] = { 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    settings_get_lora_app_eui(lora_app_eui);
    CHECK_EQUAL(0, memcmp(lora_app_eui, expected_lora_app_eui3, SETTINGS_LORA_APP_EUI_SIZE));

    cli_send("set app-eui fFffFFffFFffFFff\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t expected_lora_app_eui4[SETTINGS_LORA_APP_EUI_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    settings_get_lora_app_eui(lora_app_eui);
    CHECK_EQUAL(0, memcmp(lora_app_eui, expected_lora_app_eui4, SETTINGS_LORA_APP_EUI_SIZE));

    settings_set_lorawan_mode(true);
    cli_send("set app-eui 0FffFFffFFffFFff\r");
    STRCMP_EQUAL("ERR wrong mode" CONSOLE_EOL, rx_buffer);
}

TEST(cli_test, command_set_app_key) {
    cli_send("set app-key\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set app-key chars\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set app-key 0123456789ABCDEF000000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set app-key 0123456789ABCDEF0123456789ABCDEF000000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set app-key 0123456789ABCDEF0123456789ABCDEF\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    uint8_t experted_lora_app_key1[SETTINGS_LORA_APP_KEY_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                                                                   0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    uint8_t lora_app_key[SETTINGS_LORA_APP_KEY_SIZE];
    settings_get_lora_app_key(lora_app_key);
    CHECK_EQUAL(0, memcmp(lora_app_key, experted_lora_app_key1, SETTINGS_LORA_APP_KEY_SIZE));

    cli_send("set app-key 00000000000000000000000000000000\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t experted_lora_app_key2[SETTINGS_LORA_APP_KEY_SIZE] = { 0x00 };
    settings_get_lora_app_key(lora_app_key);
    CHECK_EQUAL(0, memcmp(lora_app_key, experted_lora_app_key2, SETTINGS_LORA_APP_KEY_SIZE));

    cli_send("set app-key fFffFFffFFffFFfffFffFFffFFffFFff\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t experted_lora_app_key3[SETTINGS_LORA_APP_KEY_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                                   0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    settings_get_lora_app_key(lora_app_key);
    CHECK_EQUAL(0, memcmp(lora_app_key, experted_lora_app_key3, SETTINGS_LORA_APP_KEY_SIZE));

    settings_set_lorawan_mode(true);
    cli_send("set app-key fFffFFffFFffFFfffFffFFffFFffFFff\r");
    STRCMP_EQUAL("ERR wrong mode" CONSOLE_EOL, rx_buffer);
}

TEST(cli_test, command_set_p2p_key) {
    cli_send("set p2p-key\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set p2p-key chars\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set p2p-key 0123456789ABCDEF000000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set p2p-key 0123456789ABCDEF0123456789ABCDEF000000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set p2p-key 0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF0123456789ABCDEF\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);

    uint8_t experted_lora_app_key1[SETTINGS_P2P_KEY_SIZE] = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                                                              0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                                                              0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF,
                                                              0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF };
    uint8_t lora_app_key[SETTINGS_P2P_KEY_SIZE];
    settings_get_p2p_key(lora_app_key);
    CHECK_EQUAL(0, memcmp(lora_app_key, experted_lora_app_key1, SETTINGS_P2P_KEY_SIZE));

    cli_send("set p2p-key 0000000000000000000000000000000000000000000000000000000000000000t\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);

    cli_send("set p2p-key 0000000000000000000000000000000000000000000000000000000000000000\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t experted_lora_app_key2[SETTINGS_P2P_KEY_SIZE] = { 0x00 };
    settings_get_p2p_key(lora_app_key);
    CHECK_EQUAL(0, memcmp(lora_app_key, experted_lora_app_key2, SETTINGS_P2P_KEY_SIZE));

    cli_send("set p2p-key fFffFFffFFffFFfffFffFFffFFffFFfffFffFFffFFffFFfffFffFFffFFffFFff\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    uint8_t experted_lora_app_key3[SETTINGS_P2P_KEY_SIZE] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
                                                              0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };
    settings_get_p2p_key(lora_app_key);
    CHECK_EQUAL(0, memcmp(lora_app_key, experted_lora_app_key3, SETTINGS_P2P_KEY_SIZE));
}

TEST(cli_test, command_enable_p2p_encryption) {
    cli_send("p2p encryption 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_get_is_p2p_encrypted());

    cli_send("p2p encryption 1\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(1, settings_get_is_p2p_encrypted());

    cli_send("p2p encryption\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_is_p2p_encrypted());

    cli_send("p2p encryption y\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_is_p2p_encrypted());

    cli_send("p2p encryption 1234567890123\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_is_p2p_encrypted());

    cli_send("p2p encryption 2\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(1, settings_get_is_p2p_encrypted());
}

TEST(cli_test, command_erase) {
    cli_send("erase\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
}

TEST(cli_test, command_goto_bootloader) {

    cli_send("\x7F\x7f");
}

TEST(cli_test, command_set_debug) {
    cli_send("debug 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_is_debug_output());

    cli_send("debug 1\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(1, settings_is_debug_output());

    cli_send("debug 12345678\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(1, settings_is_debug_output());

    cli_send("debug 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_is_debug_output());

    cli_send("debug 12345678000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(0, settings_is_debug_output());

    cli_send("debug\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(0, settings_is_debug_output());

    cli_send("debug hh\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(0, settings_is_debug_output());
}

TEST(cli_test, command_set_extended) {
    cli_send("extended 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_get_is_extended_packet());

    cli_send("extended 1\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(1, settings_get_is_extended_packet());

    cli_send("extended 12345678\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(1, settings_get_is_extended_packet());

    cli_send("extended 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_get_is_extended_packet());

    cli_send("extended 12345678000\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(0, settings_get_is_extended_packet());

    cli_send("extended\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(0, settings_get_is_extended_packet());

    cli_send("extended hh\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(0, settings_get_is_extended_packet());
}

TEST(cli_test, command_set_gnss_mode) {
    cli_send("set gnss mode 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(SETTINGS_GNSS_MODE_NORMAL, settings_get_gnss_mode());

    cli_send("set gnss mode 1\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(SETTINGS_GNSS_MODE_FITNESS, settings_get_gnss_mode());

    cli_send("set gnss mode 2\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(SETTINGS_GNSS_MODE_AVIATION, settings_get_gnss_mode());

    cli_send("set gnss mode 3\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(SETTINGS_GNSS_MODE_BALLOON, settings_get_gnss_mode());

    cli_send("set gnss mode 4\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(SETTINGS_GNSS_MODE_STATIONARY, settings_get_gnss_mode());

    cli_send("set gnss mode 5\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(SETTINGS_GNSS_MODE_STATIONARY, settings_get_gnss_mode());

    cli_send("set gnss mode cha\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(SETTINGS_GNSS_MODE_STATIONARY, settings_get_gnss_mode());

    cli_send("set gnss mode 999999999999\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(SETTINGS_GNSS_MODE_STATIONARY, settings_get_gnss_mode());
}

TEST(cli_test, command_set_tx_power) {
    cli_send("set tx 0\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_get_tx_power());

    cli_send("set tx 10\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(10, settings_get_tx_power());

    cli_send("set tx -4\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(-4, settings_get_tx_power());

    cli_send("set tx cha\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(-4, settings_get_tx_power());

    cli_send("set tx 256\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(-4, settings_get_tx_power());

    cli_send("set tx -256\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
    CHECK_EQUAL(-4, settings_get_tx_power());
}

TEST(cli_test, command_set_region) {
    cli_send("set region ?\r");
    STRNCMP_EQUAL("Available regions:" CONSOLE_EOL, rx_buffer, 18);

#if REGION_AS923 == 1
    cli_send("set region AS923\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(0, settings_get_lorawan_region_id());
#endif  //  REGION_AS923

#if REGION_AU915 == 1
    cli_send("set region AU915\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(1, settings_get_lorawan_region_id());
#endif  //  REGION_AU915

#if REGION_CN470 == 1
    cli_send("set region CN470\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(2, settings_get_lorawan_region_id());
#endif  //  REGION_CN470

#if REGION_CN779 == 1
    cli_send("set region CN779\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(3, settings_get_lorawan_region_id());
#endif  //  REGION_CN779

#if REGION_EU433 == 1
    cli_send("set region EU433\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(4, settings_get_lorawan_region_id());
#endif  //  REGION_EU433

#if REGION_EU868 == 1
    cli_send("set region EU868\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(5, settings_get_lorawan_region_id());
#endif  //  REGION_EU868

#if REGION_KR920 == 1
    cli_send("set region KR920\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(6, settings_get_lorawan_region_id());
#endif  //  REGION_KR920

#if REGION_IN865 == 1
    cli_send("set region IN865\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(7, settings_get_lorawan_region_id());
#endif  //  REGION_IN865

#if REGION_US915 == 1
    cli_send("set region US915\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(8, settings_get_lorawan_region_id());
#endif  //  REGION_US915

#if REGION_RU864 == 1
    cli_send("set region RU864\r");
    STRCMP_EQUAL("OK" CONSOLE_EOL, &rx_buffer[buffer_size - 4]);
    CHECK_EQUAL(9, settings_get_lorawan_region_id());
#endif  //  REGION_RU864

    cli_send("set region test\r");
    STRCMP_EQUAL("ERR Wrong argument" CONSOLE_EOL, rx_buffer);
}
