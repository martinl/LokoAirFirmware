#include <version.h>

#include <bsp.h>
#include <bsp_flash_layout.h>

static const uint32_t CRC_PLACEHOLDER __attribute__((section(".crc_placeholder"))) __attribute__((used)) =
    FIRMWARE_NO_CRC;

#if defined(__GNUC__)
static const version_info_block_t VERSION_BUILD_IN __attribute__((section(".version"))) __attribute__((used)) = {
#endif
    .signature = FIRMWARE_APP_SIGNATURE,
    .version_major = FIRMWARE_VERSION_MAJOR,
    .version_minor = FIRMWARE_VERSION_MINOR,
    .crc_data_len = ((uint32_t)&CRC_PLACEHOLDER) - TARGET_BASE_ADDR,
};

version_info_block_t const *version_get_app_info(void) {
    return (version_info_block_t const *)(FLASH_APP_PAGE_ADDR + FIRMWARE_INFO_OFFSET);
}

version_info_block_t const *version_get_bldr_info(void) {
    return (version_info_block_t const *)(FLASH_BLDR_PAGE_ADDR + FIRMWARE_INFO_OFFSET);
}
