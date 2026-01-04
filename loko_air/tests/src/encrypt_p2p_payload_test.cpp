#include "CppUTest/TestHarness.h"

#include "encrypt_p2p_payload.h"
#include <spy/settings_io.hpp>

#include <array>
#include <string>

TEST_GROUP(encrypt_p2p_payload_test) {
    void setup() final {
        settings_setup_io();
        settings_init(settings_get_io());
    }

    void teardown() final {
        // Nothing
    }
};

TEST(encrypt_p2p_payload_test, encryption_test) {
    enc_p2p_payload_t payload = {
        .vbat = 10,
        .reserved0 = 0,
        .lat_32bit = { 1, 2, 3, 4 },
        .lon_32bit = { 4, 3, 2, 1 },
        .speed_mps = 254,
        .alt_meters = 0x1234,
        .reserved1 = { 0 },
        .integrity = 0,
    };
    payload.integrity = enc_p2p_get_integrity_value(&payload);
    std::array<uint8_t, 16> out_encrypted;
    enc_p2p_payload_bin(out_encrypted.data(), &payload);
    std::string expected = "\x5D\x40\x23\xB6\xB8\xA3\x1B\xEF"
                           "\x53\xA4\xAE\x44\x92\xCC\x55\x9F";
    MEMCMP_EQUAL(expected.data(), reinterpret_cast<const char *>(out_encrypted.data()), out_encrypted.size());

    // Use online tool to verify the result
    // Base64, ECB, Zero, 256bits, key 00000000000000000000000000000000
}

TEST(encrypt_p2p_payload_test, integrity_calculation_2) {

    enc_p2p_payload_t payload = {
        .raw = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 0xFF}, /* #16 byte ignored in calculating */
    };

    CHECK_EQUAL(120, enc_p2p_get_integrity_value(&payload));
}

TEST(encrypt_p2p_payload_test, integrity_calculation_1) {

    enc_p2p_payload_t payload = {
        .raw = {0xFF,
                0xFF, 0xFF,
                0xFF, 0xFF,
                0xFF, 0xFF,
                0xFF, 0xFF,
                0xFF, 0xFF,
                0xFF, 0xFF,
                0xFF, 0xFF,
                0xFF}, /* #16 byte ignored in calculating */
    };

    CHECK_EQUAL(((0xFF * 15) & 0xFF), enc_p2p_get_integrity_value(&payload));
}

TEST(encrypt_p2p_payload_test, payload_size) {
    CHECK_EQUAL(16, sizeof(enc_p2p_payload_t));
}
