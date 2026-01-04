
#if defined(IT_MUST_BE_INCLUDE_ONLY_ONCE)
#    error IT_MUST_BE_INCLUDE_ONLY_ONCE
#endif
#define IT_MUST_BE_INCLUDE_ONLY_ONCE 1U

/*----------------------------------------------------------------------------*/

static void _settings_save_request_cb(void);
static void _settings_default_init_cb(settings_t *settings);

/*----------------------------------------------------------------------------*/

static void _settings_read(const uint32_t offset, void *data, const uint32_t data_size) {
    bsp_flash_settings_read(offset, data, data_size);
}

/*----------------------------------------------------------------------------*/

static void _settings_write(const uint32_t offset_in, const void *data, const uint32_t data_size) {
    bsp_flash_settings_write(offset_in, data, data_size);
}

/*----------------------------------------------------------------------------*/

static void _settings_erase(void) {
    bsp_flash_settings_erase();
}

/*----------------------------------------------------------------------------*/
static void _settings_save_request_cb(void) {
    // LOG_DEBUG("Trace: %s", __FUNCTION__);
    settings_save();
}

/*----------------------------------------------------------------------------*/

static void _settings_default_init_cb(settings_t *settings) {

    // LOG_DEBUG("Trace: %s", __FUNCTION__);

    /* Set default eui MCU UID */
    uint64_t uid = bsp_get_uid64();
    memcpy(settings->lora_dev_eui, &uid, 8);
}

/*----------------------------------------------------------------------------*/

const settings_io_t SETTINGS_IO = {
    .read = _settings_read,
    .write = _settings_write,
    .erase = _settings_erase,
    .save_request_cb = _settings_save_request_cb,
    .default_init_cb = _settings_default_init_cb,
    .page_size = BSP_FLASH_SETTINGS_PAGE_SIZE,
};
