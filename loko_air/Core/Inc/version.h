/* ----- prevent recursive inclusion ---------------------------------------- */
#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#define FIRMWARE_SIGNATURE_APP_LOKO_V1 (0x10C0A100)
#define FIRMWARE_SIGNATURE_BLD_LOKO_V1 (0x10C0B100)

#define FIRMWARE_APP_SIGNATURE  FIRMWARE_SIGNATURE_APP_LOKO_V1
#define FIRMWARE_BLDR_SIGNATURE FIRMWARE_SIGNATURE_BLD_LOKO_V1

#define FIRMWARE_DEBUG_MINOR_VER (65535U)

#define FIRMWARE_VERSION_MAJOR  1
#define FIRMWARE_VERSION_MINOR  73
#define FIRMWARE_FIELD_NOT_INIT (0x00000000)  // This value set by external tool
#define FIRMWARE_INFO_OFFSET    (0x200)
#define FIRMWARE_NO_CRC         (0x12345678UL)

typedef struct version_info_block_s {
    uint32_t signature;
    uint32_t version_major;
    uint32_t version_minor;
    uint32_t crc_data_len;
} version_info_block_t;

version_info_block_t const *version_get_app_info(void);
version_info_block_t const *version_get_bldr_info(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */
