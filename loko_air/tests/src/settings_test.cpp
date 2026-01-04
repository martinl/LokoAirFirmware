#include "CppUTest/TestHarness.h"

#include <bsp.h>

#include "settings/settings.h"
#include "settings_io.h"
#include <spy/settings_io.hpp>

#include <array>
#include <random>
#include <string>
#include <vector>

TEST_GROUP(settings_test) {
    std::minstd_rand simple_rand;

    void setup() override {
        simple_rand.seed(static_cast<unsigned int>(time(nullptr)));
        settings_setup_io();
        settings_init(settings_get_io());
    }

    void teardown() override {
        settings_reset();
    }

    void set_get_all_random() {  // Remove the 'const' qualifier from the function declaration
        std::minstd_rand::result_type test_val = (uint32_t)simple_rand();
        settings_set_id_1(static_cast<uint32_t>(test_val));
        CHECK_EQUAL(test_val, settings_get_id_1());

        test_val = simple_rand();
        settings_set_id_2(static_cast<uint32_t>(test_val));
        CHECK_EQUAL(test_val, settings_get_id_2());

        test_val = simple_rand();
        settings_set_lora_frequency_hz(static_cast<uint32_t>(test_val));
        CHECK_EQUAL(test_val, settings_get_lora_frequency_hz());

        test_val = simple_rand();
        settings_set_auto_wakeup_period_s(static_cast<uint32_t>(test_val));
        CHECK_EQUAL(test_val, settings_get_auto_wakeup_period_s());

        test_val = simple_rand();
        settings_set_gnss_trace_save_mult(static_cast<uint32_t>(test_val));
        CHECK_EQUAL(test_val, settings_get_gnss_trace_save_mult());

        test_val = simple_rand();
        settings_set_gnss_mode(static_cast<settings_gnss_mode_t>(test_val));
        CHECK_EQUAL(test_val, settings_get_gnss_mode());

        int8_t test_val_i8 = static_cast<int8_t>(simple_rand());
        settings_set_tx_power(test_val_i8);
        CHECK_EQUAL(test_val_i8, settings_get_tx_power());

        uint8_t test_val_u8 = static_cast<uint8_t>(simple_rand());
        settings_set_lorawan_region_id(test_val_u8);
        CHECK_EQUAL(test_val_u8, settings_get_lorawan_region_id());

        test_val = simple_rand() & 1;
        settings_set_lorawan_mode(test_val);
        CHECK_EQUAL(test_val, settings_get_is_lorawan_mode());

        test_val = simple_rand() & 1;
        settings_set_debug_output(test_val);
        CHECK_EQUAL(test_val, settings_is_debug_output());

        test_val = simple_rand() & 1;
        settings_set_extended_packet(test_val);
        CHECK_EQUAL(test_val, settings_get_is_extended_packet());

        uint8_t test_array_bt8[8];
        uint8_t test_array_read_back_bt8[8];

        auto test_u8 = (uint8_t)simple_rand();
        memset(test_array_bt8, test_u8, SETTINGS_LORA_DEV_EUI_SIZE);
        settings_set_lora_dev_eui(test_array_bt8);
        settings_get_lora_dev_eui(test_array_read_back_bt8);
        CHECK_EQUAL(0, memcmp(test_array_bt8, test_array_read_back_bt8, SETTINGS_LORA_DEV_EUI_SIZE));

        test_u8 = (uint8_t)simple_rand();
        memset(test_array_bt8, test_u8, SETTINGS_LORA_APP_EUI_SIZE);
        settings_set_lora_app_eui(test_array_bt8);
        settings_get_lora_app_eui(test_array_read_back_bt8);
        CHECK_EQUAL(0, memcmp(test_array_bt8, test_array_read_back_bt8, SETTINGS_LORA_APP_EUI_SIZE));

        uint8_t test_array_bt16[16];
        uint8_t test_array_read_back_bt16[16];
        test_u8 = (uint8_t)simple_rand();
        memset(test_array_bt16, test_u8, SETTINGS_LORA_APP_KEY_SIZE);
        settings_set_lora_app_key(test_array_bt16);
        settings_get_lora_app_key(test_array_read_back_bt16);
        CHECK_EQUAL(0, memcmp(test_array_bt16, test_array_read_back_bt16, SETTINGS_LORA_APP_KEY_SIZE));
    }
};

TEST(settings_test, set_get_p2p_key) {

    uint8_t test_array_read_back[SETTINGS_P2P_KEY_SIZE] = { 0 };

    uint8_t test_array_zero[SETTINGS_P2P_KEY_SIZE] = { 0 };
    settings_set_p2p_key(test_array_zero);
    settings_get_p2p_key(test_array_read_back);
    CHECK_EQUAL(0, memcmp(test_array_zero, test_array_read_back, SETTINGS_P2P_KEY_SIZE));

    uint8_t test_array_seq[SETTINGS_P2P_KEY_SIZE] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08,
                                                      0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f, 0x10 };
    settings_set_p2p_key(test_array_seq);
    settings_get_p2p_key(test_array_read_back);
    CHECK_EQUAL(0, memcmp(test_array_seq, test_array_read_back, SETTINGS_P2P_KEY_SIZE));
}

TEST(settings_test, set_get_is_p2p_encrypted) {
    bool init_state = settings_get_is_p2p_encrypted();
    settings_set_p2p_encrypted(!init_state);
    CHECK(init_state != settings_get_is_p2p_encrypted());
    settings_set_p2p_encrypted(init_state);
    CHECK(init_state == settings_get_is_p2p_encrypted());
}

TEST(settings_test, set_get_basic) {
    for (uint32_t i = 0; i < 1000; i++) {
        set_get_all_random();
    }
}

TEST(settings_test, set_get_full_page) {
    size_t FLASH_PAGE_SIZE = 2048;

    for (size_t i = 0; i < FLASH_PAGE_SIZE / sizeof(settings_t) + 1; i++) {
        settings_set_id_1(static_cast<uint32_t>(i));
        settings_save();
        settings_set_id_2(static_cast<uint32_t>(i + 1));
        settings_save();
    }

    settings_reload();
    CHECK_EQUAL(FLASH_PAGE_SIZE / sizeof(settings_t), settings_get_id_1());
    CHECK_EQUAL(FLASH_PAGE_SIZE / sizeof(settings_t) + 1, settings_get_id_2());
}

TEST(settings_test, save_reload) {
    settings_set_id_1(1);
    CHECK_EQUAL(1, settings_get_id_1());

    settings_set_id_2(2);
    CHECK_EQUAL(2, settings_get_id_2());

    settings_set_lora_frequency_hz(3);
    CHECK_EQUAL(3, settings_get_lora_frequency_hz());

    settings_set_auto_wakeup_period_s(4);
    CHECK_EQUAL(4, settings_get_auto_wakeup_period_s());

    settings_set_gnss_trace_save_mult(5);
    CHECK_EQUAL(5, settings_get_gnss_trace_save_mult());

    settings_save();

    settings_reload();

    CHECK_EQUAL(1, settings_get_id_1());
    CHECK_EQUAL(2, settings_get_id_2());
    CHECK_EQUAL(3, settings_get_lora_frequency_hz());
    CHECK_EQUAL(4, settings_get_auto_wakeup_period_s());
    CHECK_EQUAL(5, settings_get_gnss_trace_save_mult());
}

TEST(settings_test, init) {
    CHECK_FALSE(settings_init(nullptr));

    settings_io_t io;
    memset(&io, 0, sizeof(io));

    CHECK_FALSE(settings_init(&io));

    io.erase = _settings_erase;
    CHECK_FALSE(settings_init(&io));

    io.page_size = BSP_FLASH_SETTINGS_PAGE_SIZE;
    CHECK_FALSE(settings_init(&io));

    io.read = _settings_read;
    CHECK_FALSE(settings_init(&io));

    io.write = _settings_write;
    CHECK(settings_init(&io));

    settings_init(&SETTINGS_IO);
}

TEST(settings_test, init_no_default) {
    settings_io_t io = SETTINGS_IO;
    io.default_init_cb = nullptr;

    bsp_flash_settings_erase();
    CHECK_TRUE(settings_init(&io));

    set_get_all_random();

    settings_reset();

    settings_init(&SETTINGS_IO);
}

TEST(settings_test, init_no_save) {
    settings_io_t io = SETTINGS_IO;
    io.save_request_cb = nullptr;

    CHECK_TRUE(settings_init(&io));

    set_get_all_random();

    settings_init(&SETTINGS_IO);
}

TEST(settings_test, test_bad_crc) {
    size_t FLASH_PAGE_SIZE = 2048;

    settings_init(&SETTINGS_IO);

    for (size_t i = 0; i < FLASH_PAGE_SIZE / sizeof(settings_t) + 1; i++) {
        settings_set_id_1(static_cast<uint32_t>(i));
        settings_save();
        settings_set_id_2(static_cast<uint32_t>(i + 1));
        settings_save();
    }

    uint8_t brocken_data[] = { 44 };
    bsp_flash_settings_write(50, brocken_data, sizeof(brocken_data));

    settings_reload();
    CHECK_EQUAL(FLASH_PAGE_SIZE / sizeof(settings_t), settings_get_id_1());
    CHECK_EQUAL(FLASH_PAGE_SIZE / sizeof(settings_t) + 1, settings_get_id_2());
}

TEST(settings_test, dummy_call) {
    settings_print();
}

TEST(settings_test, check_default_values) {
    settings_io_t io = SETTINGS_IO;
    io.save_request_cb = nullptr;

    CHECK_TRUE(settings_init(&io));

    CHECK_EQUAL(0, settings_get_id_1());
    CHECK_EQUAL(0, settings_get_id_2());
    CHECK_EQUAL(868000000, settings_get_lora_frequency_hz());
    CHECK_EQUAL(60, settings_get_auto_wakeup_period_s());
    CHECK_EQUAL(5, settings_get_gnss_trace_save_mult());
    CHECK_EQUAL(0, settings_is_debug_output());
    CHECK_EQUAL(0, settings_get_is_lorawan_mode());
    CHECK_EQUAL(0, settings_get_is_p2p_encrypted());
    CHECK_EQUAL(0, settings_get_is_extended_packet());
    CHECK_EQUAL(0, settings_get_gnss_mode());
    CHECK_EQUAL(8, settings_get_tx_power());
    CHECK_EQUAL(5, settings_get_lorawan_region_id());
}
