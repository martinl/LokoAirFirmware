#pragma once

#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    /* Gets the version and the allowed commands supported by the current version of the protocol*/
    STM32_BOOTLOADER_CMD_GET = 0x00,
    /* Gets the protocol version */
    STM32_BOOTLOADER_CMD_GET_VERSION = 0x01,
    /* Gets the chip ID. */
    STM32_BOOTLOADER_CMD_GET_ID = 0x02,
    /* Reads up to 256 bytes of memory starting from an address specified by the application. */
    STM32_BOOTLOADER_CMD_READ_MEMORY = 0x11,
    /* Jumps to user application code located in the internal flash memory or in the SRAM. */
    STM32_BOOTLOADER_CMD_GO = 0x21,
    /*Writes up to 256 bytes to the RAM or flash memory starting from an address specified by the application. */
    STM32_BOOTLOADER_CMD_WRITE_MEMORY = 0x31,
    /* Erases from one to all the flash memory pages.*/
    STM32_BOOTLOADER_CMD_ERASE = 0x43,
    /* Erases from one to all the flash memory pages using two-byte addressing mode (available only for UART bootloader
     * v3.0 and higher).*/
    STM32_BOOTLOADER_CMD_EXTENDED_ERASE = 0x44,
    /* Generic command that allows to add new features depending on the product constraints, without adding a new
     * command for every feature. */
    STM32_BOOTLOADER_CMD_SPECIAL = 0x50,
    /* Generic command that allows the user to send more data compared to the Special command. */
    STM32_BOOTLOADER_CMD_EXTENDED_SPECIAL = 0x51,
    /*  Enables the write protection for some sectors */
    STM32_BOOTLOADER_CMD_WRITE_PROTECT = 0x63,
    /*  Disables the write protection for all flash memory sectors */
    STM32_BOOTLOADER_CMD_WRITE_UNPROTECTED = 0x73,
    /* Enables the read protection */
    STM32_BOOTLOADER_CMD_READOUT_PROTECT = 0x82,
    /* Disables the read protection */
    STM32_BOOTLOADER_CMD_READOUT_UNPROTECTED = 0x92,
    /* Computes a CRC value on a given memory area with a size multiple of 4 bytes. */
    STM32_BOOTLOADER_CMD_READOUT_GET_CHECKSUM = 0xA1,

    STM32_BOOTLOADER_ACK = 0x79,
    STM32_BOOTLOADER_NACK = 0x1F,

} stm32_bootloader_cmd_list_t;

typedef enum {
    STM32_BOOTLOADER_RESULT_OK,
    STM32_BOOTLOADER_RESULT_ERROR,
} stm32_bootloader_result_t;

typedef enum {
    STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND,
    STM32_BOOTLOADER_STATE_COMMAND_PROCCESS,
} stm32_bootloader_state_t;

typedef void (*stm32_bootloader_serial_out_t)(uint8_t const *byte, size_t size);
typedef void (*stm32_bootloader_read_mem_t)(size_t addr, uint8_t *byte, size_t size);
typedef void (*stm32_bootloader_write_mem_t)(size_t addr, uint8_t const *byte, size_t size);
typedef void (*stm32_bootloader_erase_page_t)(size_t page_number);
typedef void (*stm32_bootloader_goto_t)(size_t addr);

typedef struct {
    stm32_bootloader_serial_out_t serial_out;
    stm32_bootloader_read_mem_t read_mem;
    stm32_bootloader_write_mem_t write_mem;
    stm32_bootloader_erase_page_t erase_page;
    stm32_bootloader_goto_t goto_addr;
} stm32_bootloader_io_t;
typedef struct {
    uint8_t command;
    stm32_bootloader_state_t state;
    size_t received;
    stm32_bootloader_io_t const *io;
    uint8_t buffer[256 + 32];
} stm32_bootloader_context_t;

stm32_bootloader_result_t stm32_bootloader_host_protocol_init(stm32_bootloader_context_t *context,
                                                              stm32_bootloader_io_t const *const io);

void stm32_bootloader_host_protocol_byte_handle(stm32_bootloader_context_t *context, uint8_t byte);

#ifdef __cplusplus
}
#endif
