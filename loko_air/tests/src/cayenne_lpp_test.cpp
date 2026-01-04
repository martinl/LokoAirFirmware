#include "CppUTest/TestHarness.h"
#include "cayenne_lpp_c.h"
#include "string.h"

TEST_GROUP(cayenne_lpp_test) {
    cayenne_lpp_t lpp;
    void setup() {
        cayenne_lpp_reset(&lpp);
    }

    void teardown() {
    }
};

TEST(cayenne_lpp_test, test_cayenne_lpp_init) {
    CHECK_EQUAL(0, lpp.cursor);
}

TEST(cayenne_lpp_test, test_cayenne_lpp_reset) {
    lpp.cursor = 10;
    CHECK_EQUAL(10, lpp.cursor);
    cayenne_lpp_reset(&lpp);
    CHECK_EQUAL(0, lpp.cursor);
}

TEST(cayenne_lpp_test, test_cayenne_lpp_digital_input) {
    cayenne_lpp_add_digital_input(&lpp, 0, 10);
    CHECK_EQUAL(CAYENNE_LPP_DIGITAL_INPUT_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_DIGITAL_INPUT_SIZE] = { 0x00, 0x00, 0x0A };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_DIGITAL_INPUT_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_digital_output) {
    cayenne_lpp_add_digital_output(&lpp, 1, 25);
    CHECK_EQUAL(CAYENNE_LPP_DIGITAL_OUTPUT_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_DIGITAL_OUTPUT_SIZE] = { 0x01, 0x01, 0x19 };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_DIGITAL_OUTPUT_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_analog_input) {
    cayenne_lpp_add_analog_input(&lpp, 0, 5.2f);
    CHECK_EQUAL(CAYENNE_LPP_ANALOG_INPUT_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_ANALOG_INPUT_SIZE] = { 0x00, 0x02, 0x02, 0x07 };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_ANALOG_INPUT_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_analog_output) {
    cayenne_lpp_add_analog_output(&lpp, 1, 25.1f);
    CHECK_EQUAL(CAYENNE_LPP_ANALOG_OUTPUT_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_ANALOG_OUTPUT_SIZE] = { 0x01, 0x03, 0x09, 0xCE };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_ANALOG_OUTPUT_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_luminosity) {
    cayenne_lpp_add_luminosity(&lpp, 10, 123);
    CHECK_EQUAL(CAYENNE_LPP_LUMINOSITY_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_LUMINOSITY_SIZE] = { 0x0A, 0x65, 0x00, 0x7B };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_LUMINOSITY_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_presence) {
    cayenne_lpp_add_presence(&lpp, 5, 1);
    CHECK_EQUAL(CAYENNE_LPP_PRESENCE_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_PRESENCE_SIZE] = { 0x05, 0x66, 0x01 };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_PRESENCE_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_temperature) {
    CHECK_EQUAL(0, lpp.cursor);
    cayenne_lpp_add_temperature(&lpp, 1, -4.1f);
    CHECK_EQUAL(CAYENNE_LPP_TEMPERATURE_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_TEMPERATURE_SIZE] = { 0x01, 0x67, 0xFF, 0xD8 };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_TEMPERATURE_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_relative_humidity) {
    cayenne_lpp_add_relative_humidity(&lpp, 3, 48.5);
    CHECK_EQUAL(CAYENNE_LPP_RELATIVE_HUMIDITY_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_RELATIVE_HUMIDITY_SIZE] = { 0x03, 0x68, 0x61 };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_RELATIVE_HUMIDITY_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_barometric_pressure) {
    cayenne_lpp_add_barometric_pressure(&lpp, 10, 996.23f);
    CHECK_EQUAL(CAYENNE_LPP_BAROMETRIC_PRESSURE_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_BAROMETRIC_PRESSURE_SIZE] = { 0x0A, 0x73, 0x26, 0xEA };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_BAROMETRIC_PRESSURE_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_accelerometer) {
    CHECK_EQUAL(0, lpp.cursor);
    cayenne_lpp_add_accelerometer(&lpp, 6, 1.234f, -1.234f, 0);
    CHECK_EQUAL(CAYENNE_LPP_ACCELEROMETER_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_ACCELEROMETER_SIZE] = { 0x06, 0x71, 0x04, 0xD1, 0xFB, 0x2F, 0x00, 0x00 };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_ACCELEROMETER_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_gyrometer) {
    cayenne_lpp_add_gyrometer(&lpp, 2, 5.3f, 4.2f, -2.3f);
    CHECK_EQUAL(CAYENNE_LPP_GYROMETER_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_GYROMETER_SIZE] = { 0x02, 0x86, 0x02, 0x12, 0x01, 0xA3, 0xFF, 0x1B };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_GYROMETER_SIZE));
}

TEST(cayenne_lpp_test, test_cayenne_lpp_gps) {
    CHECK_EQUAL(0, lpp.cursor);
    cayenne_lpp_add_gps(&lpp, 1, 42.3519f, -87.9094f, 10);
    CHECK_EQUAL(CAYENNE_LPP_GPS_SIZE, lpp.cursor);
    uint8_t buffer[CAYENNE_LPP_GPS_SIZE] = { 0x01, 0x88, 0x06, 0x76, 0x5E, 0xF2, 0x96, 0x0A, 0x00, 0x03, 0xE8 };
    CHECK_EQUAL(0, memcmp(lpp.buffer, buffer, CAYENNE_LPP_GPS_SIZE));
}
