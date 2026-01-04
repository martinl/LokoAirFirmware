#include "CppUTest/TestHarness.h"

#include <bsp.h>

#include "log_io.h"

void empty_write(const uint8_t *, size_t) {
}

TEST_GROUP(log_test) {
    void setup() {
    }

    void teardown() {
        log_set_output_mask(LOG_MASK_OFF);
    }
};

TEST(log_test, normal_mode) {
    log_set_output_mask(LOG_MASK_ALL);

    LOG_INFO("Test info output");
    LOG_WARNING("Test Warning output");
    LOG_ERROR("Test Error output");

    uint8_t test_array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    LOG_DEBUG_ARRAY_BLUE("TestArray", test_array, sizeof(test_array));
}

TEST(log_test, off_mode) {
    log_set_output_mask(LOG_MASK_OFF);

    LOG_INFO("Test info output");
    LOG_WARNING("Test Warning output");
    LOG_ERROR("Test Error output");

    uint8_t test_array[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
    LOG_DEBUG_ARRAY_BLUE("TestArray", test_array, sizeof(test_array));
}

TEST(log_test, long_line) {
    log_set_output_mask(LOG_MASK_ALL);
    char test_string[LOG_MAX_MESSAGE_LENGTH] = { "There is a very long string that should be cut to LOG_MAX_MESSAGE_LENGTH 1234567890 1234567890 1234567890 1234567890 1234567890" };

    LOG_INFO("Test info output[%s]", test_string);
}

TEST(log_test, long_array) {
    log_set_output_mask(LOG_MASK_ALL);

    uint8_t test_array[256] = { 0 };
    for (uint32_t i = 0; i < sizeof(test_array); i++) {
        test_array[i] = (uint8_t)i;
    }
    LOG_DEBUG_ARRAY_BLUE("Long Array", test_array, sizeof(test_array));
}

TEST(log_test, init_fail) {
    CHECK_EQUAL(LOGGER_RESULT_ERROR, log_init(LOG_MASK_ALL, NULL));

    log_io_t no_write_io = {
        .write = NULL,
        .get_ts = NULL,
    };
    CHECK_EQUAL(LOGGER_RESULT_ERROR, log_init(LOG_MASK_ALL, &no_write_io));

    log_io_t no_get_ts_io = {
        .write = empty_write,
        .get_ts = NULL,
    };
    CHECK_EQUAL(LOGGER_RESULT_ERROR, log_init(LOG_MASK_ALL, &no_get_ts_io));

    log_init(LOG_MASK_ALL, &log_io_interface);
    log_set_output_mask(LOG_MASK_OFF);
}