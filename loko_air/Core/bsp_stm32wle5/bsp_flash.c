#include "stm32wlxx_hal.h"
#include <bsp.h>
#include <bsp_flash_layout.h>
#include <string.h>

/*----------------------------------------------------------------------------*/

static void _flash_write(const size_t offset_in, const void *data, const size_t size) {
    const size_t FLASH_WORD_SIZE = 8;
    size_t left_size = size;
    size_t offset = offset_in;

    HAL_FLASH_Unlock();

    if (offset % FLASH_WORD_SIZE) {
        LOG_ERROR("Error flash write: address not aligned to 8");
        return;
    }

    while (left_size >= FLASH_WORD_SIZE) {
        uint64_t aligned_double_word;
        memcpy(&aligned_double_word, data, FLASH_WORD_SIZE);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, offset, aligned_double_word);
        left_size -= FLASH_WORD_SIZE;
        offset += FLASH_WORD_SIZE;
        data += FLASH_WORD_SIZE;
    }

    if (left_size) {
        uint64_t read_mod_write_buff = *(uint64_t *)(offset);
        memcpy(&read_mod_write_buff, data, left_size);
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, offset, read_mod_write_buff);
    }

    HAL_FLASH_Lock();
}

/*----------------------------------------------------------------------------*/

static void _flash_erase(size_t page, size_t count) {
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase_param = {
        .TypeErase = FLASH_TYPEERASE_PAGES,
        .Page = page,
        .NbPages = count,
    };

    uint32_t page_error = 0;

    if (HAL_FLASHEx_Erase(&erase_param, &page_error) != HAL_OK) {
        // TODO Print
    }

    HAL_FLASH_Lock();
}

/*----------------------------------------------------------------------------*/

void bsp_flash_settings_read(const size_t offset, void *data, const size_t size) {
    memcpy(data, (void *)(FLASH_SETTINGS_PAGE_ADDR + offset), size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_settings_write(const size_t offset, const void *data, const size_t size) {
    _flash_write(FLASH_SETTINGS_PAGE_ADDR + offset, data, size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_settings_erase(void) {
    _flash_erase(FLASH_SETTINGS_PAGE_INDEX, FLASH_SETTINGS_PAGE_COUNT);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_gnss_trace_read(const size_t offset, void *data, const size_t size) {
    memcpy(data, (void *)(FLASH_GNSS_TRACE_PAGE_ADDR + offset), size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_gnss_trace_write(const size_t offset, const void *data, const size_t size) {
    _flash_write(FLASH_GNSS_TRACE_PAGE_ADDR + offset, data, size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_gnss_trace_erase(size_t page) {
    if (page > FLASH_GNSS_TRACE_PAGE_COUNT) {
        LOG_ERROR("Wrong erase page %u", page);
        return;
    }

    _flash_erase(FLASH_GNSS_TRACE_PAGE_INDEX + page, 1);
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

void bsp_flash_lorawan_nvm_read(const size_t offset, void *data, const size_t size) {

    memcpy(data, (void *)(FLASH_LORAWAN_NVM_PAGE_ADDR + offset), size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_lorawan_nvm_write(const size_t offset, const void *data, const size_t size) {
    _flash_write(FLASH_LORAWAN_NVM_PAGE_ADDR + offset, data, size);
}

/*----------------------------------------------------------------------------*/

void bsp_flash_lorawan_nvm_erase(void) {

    _flash_erase(FLASH_LORAWAN_NVM_PAGE_INDEX, FLASH_LORAWAN_NVM_PAGE_COUNT);
}

/*----------------------------------------------------------------------------*/

void *bsp_flash_lorawan_nvm_get_addr(void) {

    return (void *)FLASH_LORAWAN_NVM_PAGE_ADDR;
}

/*----------------------------------------------------------------------------*/

void mcu_flash_erase_app(void) {
    _flash_erase(FLASH_APP_PAGE_INDEX, FLASH_APP_PAGE_COUNT);
}

/*----------------------------------------------------------------------------*/

void mcu_flash_write_app(const size_t offset, const void *data, const size_t size) {
    _flash_write(FLASH_APP_PAGE_ADDR + offset, data, size);
}

/*----------------------------------------------------------------------------*/

void const *mcu_flash_get_app_addr(void) {
    return (const void *)FLASH_APP_PAGE_ADDR;
}

/*----------------------------------------------------------------------------*/

size_t mcu_flash_get_app_size(void) {
    return FLASH_APP_PAGE_COUNT * FLASH_PAGE_SIZE;
}

/*----------------------------------------------------------------------------*/

void mcu_flash_print_info(void) {
#if LOG_ENABLED == 1
    LOG_INFO("\r\n");

    LOG_INFO("Memory layout | Start Addr |  End Addr  |  Size, KB |");

    LOG_INFO("Bootloader    | 0x%08" PRIX32 " | 0x%08" PRIX32 " |  %08" PRIu32 " |",
             (uint32_t)FLASH_BLDR_PAGE_ADDR,
             (uint32_t)(FLASH_BLDR_PAGE_ADDR + FLASH_BLDR_SIZE - 1),
             (uint32_t)__BYTES_TO_KILOBYTES(FLASH_BLDR_SIZE));
    LOG_INFO("Application   | 0x%08" PRIX32 " | 0x%08" PRIX32 " |  %08" PRIu32 " |",
             (uint32_t)mcu_flash_get_app_addr(),
             (uint32_t)(mcu_flash_get_app_addr() + mcu_flash_get_app_size() - 1),
             (uint32_t)__BYTES_TO_KILOBYTES(mcu_flash_get_app_size()));
    LOG_INFO("LoraWAN NVM   | 0x%08" PRIX32 " | 0x%08" PRIX32 " |  %08" PRIu32 " |",
             (uint32_t)bsp_flash_lorawan_nvm_get_addr(),
             (uint32_t)(bsp_flash_lorawan_nvm_get_addr() + FLASH_LORAWAN_NVM_PAGE_SIZE - 1),
             (uint32_t)__BYTES_TO_KILOBYTES(FLASH_LORAWAN_NVM_PAGE_SIZE));
    LOG_INFO("GNSS Trace    | 0x%08" PRIX32 " | 0x%08" PRIX32 " |  %08" PRIu32 " |",
             (uint32_t)FLASH_GNSS_TRACE_PAGE_ADDR,
             (uint32_t)(FLASH_GNSS_TRACE_PAGE_ADDR + FLASH_GNSS_TRACE_PAGE_SIZE - 1),
             (uint32_t)__BYTES_TO_KILOBYTES(FLASH_GNSS_TRACE_PAGE_SIZE));
    LOG_INFO("Settings      | 0x%08" PRIX32 " | 0x%08" PRIX32 " |  %08" PRIu32 " |",
             (uint32_t)FLASH_SETTINGS_PAGE_ADDR,
             (uint32_t)(FLASH_SETTINGS_PAGE_ADDR + FLASH_SETTINGS_PAGE_SIZE - 1),
             (uint32_t)__BYTES_TO_KILOBYTES(FLASH_SETTINGS_PAGE_SIZE));

    LOG_INFO("\r\n");
#endif  // LOG_ENABLED == 1
}

/*----------------------------------------------------------------------------*/
