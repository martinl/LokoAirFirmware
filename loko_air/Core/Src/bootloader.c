/* Includes ------------------------------------------------------------------*/
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <bsp.h>
#include <crc16.h>
#include <queue/queue.h>
#include <stm32_bootloader_host_protocol.h>
#include <utils.h>
#include <version.h>

/* Private typedef -----------------------------------------------------------*/

/* Private define ------------------------------------------------------------*/
#define QUEUE_DEBUG_RX_SIZE (256 * 2) /*<! */
/* Private macro -------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/

static QUEUE(_debug_rx_queue, QUEUE_DEBUG_RX_SIZE, uint8_t);
static stm32_bootloader_context_t _context;

/* -------------------------------------------------------------------------- */

/* Called from Interrupt handler, byte receiving */
void bsp_uart_debug_byte_received(uint8_t byte) {
    static bool _is_queue_full_dbg_display = false;

    if (queue_enqueue(QHEAD(_debug_rx_queue), &byte, enqueue8) == false) {
        if (_is_queue_full_dbg_display == false) {
            LOG_ERROR("_debug_rx_queue is full");
        }

        _is_queue_full_dbg_display = true;
    } else {
        _is_queue_full_dbg_display = false;
    }
}

/* -------------------------------------------------------------------------- */

static version_info_block_t const *const _get_app_info(void) {
    return mcu_flash_get_app_addr() + FIRMWARE_INFO_OFFSET;
}

/* -------------------------------------------------------------------------- */

static bool _is_current_app_crc_corrupted(void) {

    version_info_block_t const *const app_on_flash_info = _get_app_info();

    if (app_on_flash_info->signature != FIRMWARE_APP_SIGNATURE) {
        return true;
    }

    const uint32_t CURRENT_CRC = *(uint32_t const *)(mcu_flash_get_app_addr() + app_on_flash_info->crc_data_len);

    /* is CRC not calculated, just ignore it */
    if (CURRENT_CRC == FIRMWARE_NO_CRC) {
        return false;
    }

    if (app_on_flash_info->crc_data_len < mcu_flash_get_app_size()) {

        const uint32_t CALCULATED_CRC =
            crc16_ccitt(mcu_flash_get_app_addr(), app_on_flash_info->crc_data_len, CRC16_CCITT_INIT_VAL);
        if (CALCULATED_CRC == CURRENT_CRC) {
            return false;
        }
    }

    return true;
}

/* --------------------------------------------------------------------------- */

static void _read_mem(size_t addr, uint8_t *byte, size_t size) {
    static uint8_t _read_info[32] = { 0 };

    if (*_read_info == 0) {
        if (version_get_app_info()->signature == FIRMWARE_APP_SIGNATURE) {
            snprintf((char *)_read_info,
                     sizeof(_read_info),
                     "Loko bootloader App v%" PRIu32 ".%" PRIu32,
                     version_get_app_info()->version_major,
                     version_get_app_info()->version_minor);
        } else {
            snprintf((char *)_read_info, sizeof(_read_info), "Loko bootloader App not found");
        }
    }
    const size_t APP_ADDR = (size_t)mcu_flash_get_app_addr();
    const size_t BLDR_ADDR = 0x08000000;
    const size_t SETTINGS_START_ADDR = 0x0803F800;  // TODO Remove magic numbers
    const size_t SETTINGS_END_ADDR = 0x08040000;
    const bool is_bootloader_range = ((addr >= BLDR_ADDR) && (addr < APP_ADDR));
    const bool is_settings_range = ((addr >= SETTINGS_START_ADDR) && (addr < SETTINGS_END_ADDR));
    if (is_bootloader_range || is_settings_range) {
        for (size_t i = 0; i < size; i++) {
            byte[i] = _read_info[i % sizeof(_read_info)];
        }
        return;
    }

    memcpy(byte, (uint8_t *)addr, size);
}

/* --------------------------------------------------------------------------- */

static void _write_mem(size_t addr, uint8_t const *byte, size_t size) {
    const size_t APP_ADDR = (size_t)mcu_flash_get_app_addr();
    if (addr < APP_ADDR) {
        return;
    }
    if ((addr + size) > (APP_ADDR + mcu_flash_get_app_size())) {
        return;
    }

    mcu_flash_write_app(addr - APP_ADDR, byte, size);
}

/* --------------------------------------------------------------------------- */

static void _erase_page(size_t page_number) {
    if ((page_number == (0x8000 / BSP_FLASH_SETTINGS_PAGE_SIZE)) || (page_number == 0xFF)) {
        mcu_flash_erase_app();
    }
}

/* --------------------------------------------------------------------------- */

static void _goto_addr(size_t addr) {
    (void)addr;
    bsp_delay_ms(100);
    bsp_system_reset();
}

/* --------------------------------------------------------------------------- */

static stm32_bootloader_io_t const BLDR_PROTOCOL_IO = {
    .serial_out = bsp_uart_debug_write,
    .read_mem = _read_mem,
    .write_mem = _write_mem,
    .erase_page = _erase_page,
    .goto_addr = _goto_addr,
};

/* --------------------------------------------------------------------------- */

int main(void) {

    bsp_init();
    bsp_clock_init();
    bsp_gpio_init();
    bsp_rtc_init();

#if (CRC16_MODE == CRC_RAM_TABLE)
    crc16_generate_table();
#endif /* CRC16_MODE */

    bool is_bootloader_requested = (INTER_TARGET_MAILBOX_GET() == INTER_TARGET_MAILBOX_CMD_STAY_IN_BOOTLOADER);
    bsp_rtc_store_write_reg(0, 0);

    if ((is_bootloader_requested == false) && (_is_current_app_crc_corrupted() == false)) {
        bsp_launch_app();
    }

    bsp_clock_switch(BSP_CLOCK_CORE_16_MHZ);

    queue_init(QHEAD(_debug_rx_queue), QUEUE_DEBUG_RX_SIZE);
    bsp_uart_debug_init();
    stm32_bootloader_host_protocol_init(&_context, &BLDR_PROTOCOL_IO);
    uint32_t blink_timestamp_ms = bsp_get_ticks();
    uint32_t button_pressed_duration_ms = 0;
    while (bsp_gpio_is_usb_charger_connect()) {
        if (bsp_get_ticks() - blink_timestamp_ms > 500) {
            blink_timestamp_ms = bsp_get_ticks();
            bsp_gpio_led_toggle();
        }

        /* Parse debug stream */
        uint8_t queue_item = 0;
        while (queue_dequeue(QHEAD(_debug_rx_queue), &queue_item, dequeue8) == true) {
            stm32_bootloader_host_protocol_byte_handle(&_context, queue_item);
        }

        if (bsp_gpio_is_button_pressed() == false) {
            button_pressed_duration_ms = bsp_get_ticks();
        } else {
            if (bsp_get_ticks() - button_pressed_duration_ms > 3000) {
                bsp_system_reset();
            }
        }
    }
#if CONFIG_BSP_USE_RTC == 1
    bsp_rtc_wakeup_deactivate();
#endif  // CONFIG_BSP_USE_RTC == 1
    bsp_lp_shutdown(WAKEUP_SOURCE_BUTTON_1_MASK | WAKEUP_SOURCE_5V_DETECT_MASK, 0);
}
