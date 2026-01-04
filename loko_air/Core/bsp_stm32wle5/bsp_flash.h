#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define BSP_FLASH_SETTINGS_PAGE_SIZE (2048)

void bsp_flash_settings_read(const size_t offset, void *data, const size_t size);
void bsp_flash_settings_write(const size_t offset, const void *data, const size_t size);
void bsp_flash_settings_erase(void);

void bsp_flash_gnss_trace_read(const size_t offset, void *data, const size_t size);
void bsp_flash_gnss_trace_write(const size_t offset, const void *data, const size_t size);
void bsp_flash_gnss_trace_erase(size_t page);
size_t bsp_flash_get_gnss_trace_page_count(void);
size_t bsp_flash_get_gnss_trace_page_size(void);

void bsp_flash_lorawan_nvm_read(const size_t offset, void *data, const size_t size);
void bsp_flash_lorawan_nvm_write(const size_t offset, const void *data, const size_t size);
void bsp_flash_lorawan_nvm_erase(void);
void *bsp_flash_lorawan_nvm_get_addr(void);

void mcu_flash_erase_app(void);
void mcu_flash_write_app(const size_t offset, const void *data, const size_t size);
void const *mcu_flash_get_app_addr(void);
size_t mcu_flash_get_app_size(void);

void mcu_flash_print_info(void);

#ifdef __cplusplus
}
#endif
