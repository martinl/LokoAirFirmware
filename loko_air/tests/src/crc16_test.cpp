#include "CppUTest/TestHarness.h"

#include <crc16.h>

TEST_GROUP(crc16_test) {
    void setup() {
#if (CRC16_MODE == CRC_RAM_TABLE)
        crc16_generate_table();
#endif /* CRC16_MODE */
    }

    void teardown() {
    }
};

TEST(crc16_test, pass_me) {
    uint16_t crc_result = crc16_ccitt((uint8_t *)"123456789", 9, CRC16_CCITT_INIT_VAL);
    CHECK_EQUAL(0x29B1, crc_result);
}
