#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define ENC_BLOCK_SIZE 16

typedef struct PACKED {
    union {
        struct PACKED {
            uint8_t vbat      : 4; /* (n + 27) * 0.1 */
            uint8_t reserved0 : 4;
            uint8_t lat_32bit[4];
            uint8_t lon_32bit[4];
            uint8_t speed_mps;
            uint16_t alt_meters;
            uint8_t reserved1[3];
            uint8_t integrity;
        };
        uint8_t raw[ENC_BLOCK_SIZE];
    };
} __packed enc_p2p_payload_t;

uint8_t enc_p2p_get_integrity_value(enc_p2p_payload_t const *payload);
void enc_p2p_payload_bin(uint8_t encrypted[ENC_BLOCK_SIZE], enc_p2p_payload_t const *payload);

#ifdef __cplusplus
}
#endif /* __cplusplus */
