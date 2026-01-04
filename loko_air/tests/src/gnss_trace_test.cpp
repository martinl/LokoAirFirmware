#include "CppUTest/TestHarness.h"

#include <gnss_trace/gnss_trace.h>
#include <string.h>

TEST_GROUP(gnss_trace_test) {
    gtrace_t gtrace;
    void setup() {
    }

    void teardown() {
    }
};

TEST(gnss_trace_test, basic_add_get_full_range) {
    gtrace_init(&gtrace);
    gtrace_erase_all(&gtrace);

    const size_t SAVE_REC_COUNT = gtrace.max_records;

    CHECK_EQUAL(2048 * 2 / sizeof(gtrace_record_t), SAVE_REC_COUNT);

    for (size_t i = 0; i < SAVE_REC_COUNT; i++) {
        gtrace_record_t record;
        memset(&record, (uint8_t)i, sizeof(record));

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_add(&gtrace, &record));
    }

    CHECK_EQUAL(SAVE_REC_COUNT, gtrace_get_record_count(&gtrace));

    for (size_t i = 0; i < SAVE_REC_COUNT; i++) {
        gtrace_record_t expected_record;
        memset(&expected_record, (uint8_t)i, sizeof(expected_record));

        gtrace_record_t record;

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_get_record(&gtrace, i, &record));

        CHECK_EQUAL(0, memcmp(&expected_record, &record, sizeof(record) - 1 /* minus checksum size */));
    }
}

TEST(gnss_trace_test, basic_add_get_full_range_plus_one) {
    gtrace_init(&gtrace);
    gtrace_erase_all(&gtrace);

    const size_t SAVE_REC_COUNT = gtrace.max_records;

    for (size_t i = 0; i < SAVE_REC_COUNT + 1; i++) {
        gtrace_record_t record;
        memset(&record, (uint8_t)i, sizeof(record));

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_add(&gtrace, &record));
    }

    /* One page should be dropped */

    const size_t NEW_SAVE_REC_COUNT = gtrace_get_record_count(&gtrace);
    CHECK_EQUAL(SAVE_REC_COUNT / 2 + 1, NEW_SAVE_REC_COUNT);

    for (size_t i = 0; i < NEW_SAVE_REC_COUNT; i++) {
        gtrace_record_t expected_record;
        uint8_t test_val = (uint8_t)(i + SAVE_REC_COUNT / 2);
        memset(&expected_record, test_val, sizeof(expected_record));

        gtrace_record_t record;

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_get_record(&gtrace, i, &record));

        CHECK_EQUAL(0, memcmp(&expected_record, &record, sizeof(record) - 1 /* minus checksum size */));
    }
}

TEST(gnss_trace_test, check_range) {
    gtrace_init(&gtrace);
    gtrace_erase_all(&gtrace);

    CHECK_EQUAL(0, gtrace_get_record_count(&gtrace));

    gtrace_record_t record;
    CHECK_EQUAL(GTRACE_RESULT_ERROR, gtrace_get_record(&gtrace, 0, &record));
    CHECK_EQUAL(GTRACE_RESULT_ERROR, gtrace_get_record(&gtrace, 9999, &record));

    memset(&record, (uint8_t)0x11, sizeof(record));
    CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_add(&gtrace, &record));

    CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_get_record(&gtrace, 0, &record));
    CHECK_EQUAL(GTRACE_RESULT_ERROR, gtrace_get_record(&gtrace, 9999, &record));
}

TEST(gnss_trace_test, check_reinit_after_one) {
    gtrace_init(&gtrace);
    gtrace_erase_all(&gtrace);

    CHECK_EQUAL(0, gtrace_get_record_count(&gtrace));

    gtrace_record_t record;
    memset(&record, (uint8_t)0x11, sizeof(record));
    CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_add(&gtrace, &record));

    gtrace_init(&gtrace);
    CHECK_EQUAL(1, gtrace_get_record_count(&gtrace));
    CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_get_record(&gtrace, 0, &record));
    CHECK_EQUAL(GTRACE_RESULT_ERROR, gtrace_get_record(&gtrace, 1, &record));
}

TEST(gnss_trace_test, check_reinit_after_half_storage) {
    gtrace_init(&gtrace);
    gtrace_erase_all(&gtrace);

    CHECK_EQUAL(0, gtrace_get_record_count(&gtrace));

    const size_t SAVE_REC_COUNT = gtrace.max_records;

    for (size_t i = 0; i < SAVE_REC_COUNT / 2; i++) {
        gtrace_record_t record;
        memset(&record, (uint8_t)i, sizeof(record));

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_add(&gtrace, &record));
    }

    gtrace_init(&gtrace);
    CHECK_EQUAL(SAVE_REC_COUNT / 2, gtrace_get_record_count(&gtrace));

    for (size_t i = 0; i < SAVE_REC_COUNT / 2; i++) {
        gtrace_record_t expected_record;
        uint8_t test_val = (uint8_t)(i);
        memset(&expected_record, test_val, sizeof(expected_record));

        gtrace_record_t record;

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_get_record(&gtrace, i, &record));

        CHECK_EQUAL(0, memcmp(&expected_record, &record, sizeof(record) - 1 /* minus checksum size */));
    }
}

TEST(gnss_trace_test, check_reinit_after_half_storage_and_one) {
    gtrace_init(&gtrace);
    gtrace_erase_all(&gtrace);

    CHECK_EQUAL(0, gtrace_get_record_count(&gtrace));

    const size_t SAVE_REC_COUNT = gtrace.max_records;

    for (size_t i = 0; i < SAVE_REC_COUNT / 2 + 1; i++) {
        gtrace_record_t record;
        memset(&record, (uint8_t)i, sizeof(record));

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_add(&gtrace, &record));
    }

    gtrace_init(&gtrace);
    CHECK_EQUAL(SAVE_REC_COUNT / 2 + 1, gtrace_get_record_count(&gtrace));

    for (size_t i = 0; i < SAVE_REC_COUNT / 2 + 1; i++) {
        gtrace_record_t expected_record;
        uint8_t test_val = (uint8_t)(i);
        memset(&expected_record, test_val, sizeof(expected_record));

        gtrace_record_t record;

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_get_record(&gtrace, i, &record));

        CHECK_EQUAL(0, memcmp(&expected_record, &record, sizeof(record) - 1 /* minus checksum size */));
    }
}

TEST(gnss_trace_test, check_reinit_after_full) {
    gtrace_init(&gtrace);
    gtrace_erase_all(&gtrace);

    CHECK_EQUAL(0, gtrace_get_record_count(&gtrace));

    const size_t SAVE_REC_COUNT = gtrace.max_records;

    for (size_t i = 0; i < SAVE_REC_COUNT; i++) {
        gtrace_record_t record;
        memset(&record, (uint8_t)i, sizeof(record));

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_add(&gtrace, &record));
    }

    gtrace_init(&gtrace);
    CHECK_EQUAL(SAVE_REC_COUNT, gtrace_get_record_count(&gtrace));

    for (size_t i = 0; i < SAVE_REC_COUNT; i++) {
        gtrace_record_t expected_record;
        uint8_t test_val = (uint8_t)(i);
        memset(&expected_record, test_val, sizeof(expected_record));

        gtrace_record_t record;

        CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_get_record(&gtrace, i, &record));

        CHECK_EQUAL(0, memcmp(&expected_record, &record, sizeof(record) - 1 /* minus checksum size */));
    }

    gtrace_record_t record;
    memset(&record, (uint8_t)0x11, sizeof(record));
    CHECK_EQUAL(GTRACE_RESULT_OK, gtrace_add(&gtrace, &record));

    CHECK_EQUAL(SAVE_REC_COUNT / 2 + 1, gtrace_get_record_count(&gtrace));
    gtrace_init(&gtrace);
    CHECK_EQUAL(SAVE_REC_COUNT / 2 + 1, gtrace_get_record_count(&gtrace));
    gtrace_init(&gtrace);
}
