#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void bsp_flash_settings_read(const size_t offset, void *data, const size_t data_size);
void bsp_flash_settings_write(const size_t offset, const void *data, const size_t size);
void bsp_flash_settings_erase(void);

void bsp_flash_gnss_trace_read(const size_t offset, void *data, const size_t data_size);
void bsp_flash_gnss_trace_write(const size_t offset, const void *data, const size_t size);
void bsp_flash_gnss_trace_erase(size_t page);
size_t bsp_flash_get_gnss_trace_page_count(void);
size_t bsp_flash_get_gnss_trace_page_size(void);

void bsp_flash_lorawan_nvm_read(const size_t offset, void *data, const size_t size);
void bsp_flash_lorawan_nvm_write(const size_t offset, const void *data, const size_t size);
void bsp_flash_lorawan_nvm_erase(void);
void *bsp_flash_lorawan_nvm_get_addr(void);

void bsp_uart_debug_write(uint8_t const *data, size_t size);
size_t bsp_uart_debug_get_buffer(char *out_data, size_t size);
void bsp_uart_debug_drop_buffer(void);

uint32_t bsp_get_ticks(void);
void bsp_fake_forward_ticks_ms(uint32_t ms);

void bsp_system_reset(void);

uint64_t bsp_get_uid64(void);

void bsp_delay_ms(uint32_t ms);

#define BSP_FLASH_SETTINGS_PAGE_SIZE (2048)

#ifdef __cplusplus
}
#endif /* __cplusplus */
