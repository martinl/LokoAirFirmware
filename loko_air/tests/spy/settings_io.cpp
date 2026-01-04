#include "CppUTest/TestHarness.h"

#include <string.h>

#include "settings/settings.h"

/*----------------------------------------------------------------------------*/

#define FLASH_PAGE_SIZE 2048

settings_io_t SETTINGS_IO = {
    .read = NULL,
    .write = NULL,
    .erase = NULL,
    .save_request_cb = NULL,
    .default_init_cb = NULL,
    .page_size = FLASH_PAGE_SIZE,
};

/*----------------------------------------------------------------------------*/

static uint8_t flash_page[FLASH_PAGE_SIZE] = { 0 };

/*----------------------------------------------------------------------------*/

static void _settings_read(const size_t offset, void *data, const size_t size) {
    memcpy(data, &flash_page[offset], size);
}

/*----------------------------------------------------------------------------*/

static void _settings_write(const size_t offset, const void *data, const size_t size) {
    memcpy(&flash_page[offset], data, size);
}

/*----------------------------------------------------------------------------*/

static void _settings_erase(void) {
    memset(flash_page, 0xFF, FLASH_PAGE_SIZE);
}

/*----------------------------------------------------------------------------*/

void settings_setup_io() {

    SETTINGS_IO.erase = _settings_erase;
    SETTINGS_IO.page_size = FLASH_PAGE_SIZE;
    SETTINGS_IO.read = _settings_read;
    SETTINGS_IO.write = _settings_write;
    SETTINGS_IO.save_request_cb = NULL;
    SETTINGS_IO.default_init_cb = NULL;
}

/*----------------------------------------------------------------------------*/

settings_io_t *settings_get_io(void) {

    return &SETTINGS_IO;
}

/*----------------------------------------------------------------------------*/

void settings_set_noise_on_page(void) {
    for (size_t i = 0; i < FLASH_PAGE_SIZE; i++) {
        flash_page[i] = (uint8_t)rand();
    }
}

/*----------------------------------------------------------------------------*/
