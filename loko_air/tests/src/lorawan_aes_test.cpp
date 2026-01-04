#include "CppUTest/TestHarness.h"

#include "Crypto/lorawan_aes.h"
#include <string.h>

#include <array>
#include <string>
#include <vector>

TEST_GROUP(lorawan_aes_test) {

    const std::array<uint8_t, N_BLOCK> key = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
                                               0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f };
    lorawan_aes_context ctx;
    void setup() final {
        CHECK_EQUAL(0, lorawan_aes_set_key(key.data(), (length_type)key.size(), &ctx));
    }

    void teardown() final {
        // nothing to do
    }
};

TEST(lorawan_aes_test, pass_me) {

    std::string text = "0123456789abcdef";
    std::string encrypted_text;
    return_type result = lorawan_aes_encrypt((uint8_t *)text.data(), (uint8_t *)encrypted_text.data(), &ctx);
    CHECK_EQUAL(0, result);

#if defined(AES_DEC_PREKEYED)
    std::string decrypted_text(N_BLOCK + 1, '\0');
    result = lorawan_aes_decrypt((uint8_t *)encrypted_text.data(), (uint8_t *)decrypted_text.data(), &ctx);
    CHECK_EQUAL(0, result);

    STRCMP_EQUAL(text.data(), decrypted_text.data());
#endif  // AES_DEC_PREKEYED
}

TEST(lorawan_aes_test, several_blocks) {

    std::string text = "12,123,123.12345,321.54321,4200";
    const size_t BLOCK_COUNT = (text.length() + N_BLOCK - 1) / N_BLOCK;

    std::vector<uint8_t> encrypted_text(N_BLOCK * BLOCK_COUNT, 0);
    for (size_t i = 0; i < BLOCK_COUNT; i++) {
        return_type result =
            lorawan_aes_encrypt((uint8_t *)text.data() + i * N_BLOCK, &encrypted_text[i * N_BLOCK], &ctx);
        CHECK_EQUAL(0, result);
    }

#if defined(AES_DEC_PREKEYED)
    std::vector<uint8_t> decrypted_text(N_BLOCK * BLOCK_COUNT + 1, 0);
    for (size_t i = 0; i < BLOCK_COUNT; i++) {
        return_type result = lorawan_aes_decrypt(&encrypted_text[i * N_BLOCK], &decrypted_text[i * N_BLOCK], &ctx);
        CHECK_EQUAL(0, result);
    }

    decrypted_text.back() = '\0';
    STRCMP_EQUAL(text.data(), reinterpret_cast<const char *>(decrypted_text.data()));
#endif  // AES_DEC_PREKEYED
}
