#include "bsp.h"
#include <string.h>

#define FLASH_PAGE_SIZE 2048

// #define FLASH_SETTINGS_PAGE_INDEX (FLASH_PAGE_NB - 1U)
// #define FLASH_SETTINGS_PAGE_ADDR  (FLASH_SETTINGS_PAGE_INDEX * FLASH_PAGE_SIZE + FLASH_BASE)
#define FLASH_SETTINGS_PAGE_COUNT (1U)

// #define FLASH_GNSS_TRACE_PAGE_INDEX (FLASH_SETTINGS_PAGE_INDEX - FLASH_GNSS_TRACE_PAGE_COUNT)
// #define FLASH_GNSS_TRACE_PAGE_ADDR  (FLASH_GNSS_TRACE_PAGE_INDEX * FLASH_PAGE_SIZE + FLASH_BASE)
#define FLASH_GNSS_TRACE_PAGE_COUNT (2U)

#define FLASH_LORAWAN_NVM_PAGE_COUNT (1U)

/*----------------------------------------------------------------------------*/
static uint8_t _settings_fake_region[FLASH_PAGE_SIZE];

void bsp_flash_settings_read(const size_t offset, void *data, const size_t data_size) {
    memcpy(data, &_settings_fake_region[offset], data_size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_settings_write(const size_t offset, const void *data, const size_t size) {
    memcpy(&_settings_fake_region[offset], data, size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_settings_erase(void) {
    memset(_settings_fake_region, 0xFF, FLASH_SETTINGS_PAGE_COUNT * FLASH_PAGE_SIZE);
}

/*----------------------------------------------------------------------------*/

static uint8_t _gnss_trace_fake_region[FLASH_PAGE_SIZE * FLASH_GNSS_TRACE_PAGE_COUNT];

void bsp_flash_gnss_trace_read(const size_t offset, void *data, const size_t data_size) {
    memcpy(data, &_gnss_trace_fake_region[offset], data_size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_gnss_trace_write(const size_t offset, const void *data, const size_t size) {
    memcpy(&_gnss_trace_fake_region[offset], data, size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_gnss_trace_erase(size_t page) {
    if (page > FLASH_GNSS_TRACE_PAGE_COUNT) {
        LOG_ERROR("Wrong erase page %u", page);
        return;
    }

    memset(&_gnss_trace_fake_region[page * FLASH_PAGE_SIZE], 0xFF, FLASH_PAGE_SIZE);
}

/*----------------------------------------------------------------------------*/

size_t bsp_flash_get_gnss_trace_page_count(void) {
    return FLASH_GNSS_TRACE_PAGE_COUNT;
}

/*----------------------------------------------------------------------------*/

size_t bsp_flash_get_gnss_trace_page_size(void) {
    return FLASH_PAGE_SIZE;
}

/*----------------------------------------------------------------------------*/

static uint8_t _lorawan_fake_region[FLASH_PAGE_SIZE * FLASH_LORAWAN_NVM_PAGE_COUNT];
void bsp_flash_lorawan_nvm_read(const size_t offset, void *data, const size_t size) {

    memcpy(data, &_lorawan_fake_region[offset], size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_lorawan_nvm_write(const size_t offset, const void *data, const size_t size) {

    memcpy(&_lorawan_fake_region[offset], data, size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_lorawan_nvm_erase(void) {

    memset(_lorawan_fake_region, 0, FLASH_LORAWAN_NVM_PAGE_COUNT);
}

/*----------------------------------------------------------------------------*/

void *bsp_flash_lorawan_nvm_get_addr(void) {

    return (void *)_lorawan_fake_region;
}

/*----------------------------------------------------------------------------*/
