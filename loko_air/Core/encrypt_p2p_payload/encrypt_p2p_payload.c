#include "encrypt_p2p_payload.h"

#include "Crypto/lorawan_aes.h"
#include <bsp.h>
#include <settings/settings.h>
#include <utils.h>

STATIC_ASSERT((sizeof(enc_p2p_payload_t)) == 16);
STATIC_ASSERT((sizeof(enc_p2p_payload_t)) == ENC_BLOCK_SIZE);

/* -------------------------------------------------------------------------- */

uint8_t enc_p2p_get_integrity_value(enc_p2p_payload_t const *payload) {

    uint8_t const *data = (uint8_t const *)payload;
    uint8_t checksum = 0;
    for (size_t i = 0; i < sizeof(enc_p2p_payload_t) - 1; i++) {
        checksum += data[i];
    }

    return checksum;
}

/* -------------------------------------------------------------------------- */

void enc_p2p_payload_bin(uint8_t encrypted[ENC_BLOCK_SIZE], enc_p2p_payload_t const *payload) {

    uint8_t p2p_key[SETTINGS_P2P_KEY_SIZE];
    settings_get_p2p_key(p2p_key);

    lorawan_aes_encrypt_256(payload->raw, encrypted, p2p_key, p2p_key);
}

/* -------------------------------------------------------------------------- */
