#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>

#define CRC_ROM_TABLE 0U
#define CRC_RAM_TABLE 1U
#define CRC_NO_TABLE  2U

#define CRC16_MODE CRC_ROM_TABLE

#define CRC16_CCITT_INIT_VAL 0xFFFF

uint16_t crc16_offset_ccitt(const uint16_t crc_start_value, const uint8_t *data, uint32_t data_size);
uint16_t crc16_ccitt(const uint8_t block[], uint32_t blockLength, const uint16_t crc);

#if (CRC16_MODE == CRC_RAM_TABLE)
void crc16_generate_table(void);
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */
