#include "stm32_bootloader_host_protocol.h"

/* -------------------------------------------------------------------------- */

static const uint8_t STM32_BOOTLOADER_VERSION = 0x10;

/* -------------------------------------------------------------------------- */

static bool _is_command_checksum_valid(uint8_t const *rx_buffer) {
    return (rx_buffer[0] + rx_buffer[1]) == 0xFF;
}

/* -------------------------------------------------------------------------- */

static bool _is_data_checksum_valid(uint8_t const *rx_buffer, size_t size) {
    uint8_t checksum = 0;
    for (size_t i = 0; i < size; i++) {
        checksum ^= rx_buffer[i];
    }
    return checksum == 0;
}

/* -------------------------------------------------------------------------- */

static uint32_t _array_to_u32(uint8_t const *array) {
    return (array[0] << 24) | (array[1] << 16) | (array[2] << 8) | array[3];
}

/* -------------------------------------------------------------------------- */

static bool _is_supported_cmd(uint8_t cmd) {
    // clang-format off
    return (cmd == STM32_BOOTLOADER_CMD_GET) ||
           (cmd == STM32_BOOTLOADER_CMD_GET_VERSION) ||
           (cmd == STM32_BOOTLOADER_CMD_GET_ID) ||
           (cmd == STM32_BOOTLOADER_CMD_READ_MEMORY) ||
           (cmd == STM32_BOOTLOADER_CMD_GO) ||
           (cmd == STM32_BOOTLOADER_CMD_WRITE_MEMORY) ||
           (cmd == STM32_BOOTLOADER_CMD_ERASE) ||
           0;
    // clang-format on
}
/* -------------------------------------------------------------------------- */

static void _send_ack_nack(stm32_bootloader_context_t const *context, bool is_ack) {
    uint8_t xck = is_ack ? STM32_BOOTLOADER_ACK : STM32_BOOTLOADER_NACK;
    context->io->serial_out(&xck, sizeof(xck));
}

/* -------------------------------------------------------------------------- */

static void _send_cmd_response(stm32_bootloader_context_t *context) {
    switch (context->command) {
        case STM32_BOOTLOADER_CMD_GET: {
            uint8_t const response[] = { 6,
                                         STM32_BOOTLOADER_VERSION,
                                         STM32_BOOTLOADER_CMD_GET,
                                         STM32_BOOTLOADER_CMD_GET_VERSION,
                                         STM32_BOOTLOADER_CMD_GET_ID,
                                         STM32_BOOTLOADER_CMD_READ_MEMORY,
                                         STM32_BOOTLOADER_CMD_GO,
                                         STM32_BOOTLOADER_CMD_WRITE_MEMORY,
                                         STM32_BOOTLOADER_CMD_ERASE,
                                         STM32_BOOTLOADER_ACK };
            context->io->serial_out(response, sizeof(response));
            context->state = STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND;
        } break;
        case STM32_BOOTLOADER_CMD_GET_VERSION: {
            uint8_t const response[] = { STM32_BOOTLOADER_VERSION, 0x00, 0x00, STM32_BOOTLOADER_ACK };
            context->io->serial_out(response, sizeof(response));
            context->state = STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND;
        } break;
        case STM32_BOOTLOADER_CMD_GET_ID: {
            uint8_t const response[] = { 0x01, 0x04, 0x97, STM32_BOOTLOADER_ACK };
            context->io->serial_out(response, sizeof(response));
            context->state = STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND;
        } break;

        default:
            break;
    }
}

/* -------------------------------------------------------------------------- */

static void _reset_rx_context(stm32_bootloader_context_t *context) {
    context->state = STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND;
    context->received = 0;
}

/* -------------------------------------------------------------------------- */

static void _command_check(stm32_bootloader_context_t *context) {

    if ((context->received == 1) && (context->buffer[0] == 0x7F)) {
        _reset_rx_context(context);
        _send_ack_nack(context, true);
        return;
    }

    if (context->received >= 2) {
        if (_is_command_checksum_valid(context->buffer) == false) {
            _reset_rx_context(context);
            _send_ack_nack(context, false);
            return;
        }

        context->command = context->buffer[0];
        bool is_supported_cmd = _is_supported_cmd(context->command);

        _reset_rx_context(context);
        _send_ack_nack(context, is_supported_cmd);
        if (is_supported_cmd == true) {
            context->state = STM32_BOOTLOADER_STATE_COMMAND_PROCCESS;
            _send_cmd_response(context);
        }
    }
}

/* -------------------------------------------------------------------------- */

stm32_bootloader_result_t stm32_bootloader_host_protocol_init(stm32_bootloader_context_t *context,
                                                              stm32_bootloader_io_t const *const io) {

    if ((context == NULL) || (io == NULL)) {
        return STM32_BOOTLOADER_RESULT_ERROR;
    }

    if ((io->erase_page == NULL) || (io->read_mem == NULL) || (io->serial_out == NULL) || (io->write_mem == NULL) ||
        (io->goto_addr == NULL)) {
        return STM32_BOOTLOADER_RESULT_ERROR;
    }

    _reset_rx_context(context);
    context->io = io;
    return STM32_BOOTLOADER_RESULT_OK;
}

/* -------------------------------------------------------------------------- */

void stm32_bootloader_host_protocol_byte_handle(stm32_bootloader_context_t *context, uint8_t byte) {

    if (context->received >= sizeof(context->buffer)) {
        context->received = 0;
    }
    context->buffer[context->received] = byte;
    context->received++;

    if (STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND == context->state) {
        _command_check(context);
        return;
    }

    switch (context->command) {
        case STM32_BOOTLOADER_CMD_READ_MEMORY:
            if (context->received == 5) {
                if (_is_data_checksum_valid(context->buffer, 5) == true) {
                    _send_ack_nack(context, true);
                } else {
                    _send_ack_nack(context, false);
                    _reset_rx_context(context);
                }
            } else if (context->received == 7) {
                if (_is_command_checksum_valid(&context->buffer[5]) == false) {
                    _send_ack_nack(context, false);
                } else {
                    _send_ack_nack(context, true);

                    size_t size = context->buffer[5];
                    if (size > 0) {
                        size++;
                        uint32_t address = _array_to_u32(context->buffer);
                        uint8_t read_buffer[256];
                        context->io->read_mem(address, read_buffer, size);
                        context->io->serial_out(read_buffer, size);
                    }
                }
                _reset_rx_context(context);
            }
            break;
        case STM32_BOOTLOADER_CMD_ERASE:
            if ((context->received == 2) && (context->buffer[0] == 0xFF)) {
                context->io->erase_page(0xFF);
                _send_ack_nack(context, true);
                _reset_rx_context(context);
            } else if (context->received == ((size_t)context->buffer[0] + 3)) {
                size_t page_count = context->buffer[0] + 1;
                size_t payload_size = page_count + 1 + 1;
                if (_is_data_checksum_valid(context->buffer, payload_size) == true) {
                    for (size_t i = 0; i < page_count; i++) {
                        context->io->erase_page(context->buffer[1 + i]);
                    }
                    _send_ack_nack(context, true);
                } else {
                    _send_ack_nack(context, false);
                }
                _reset_rx_context(context);
            }
            break;
        case STM32_BOOTLOADER_CMD_WRITE_MEMORY:
            if (context->received == 5) {
                if (_is_data_checksum_valid(context->buffer, 5) == true) {
                    _send_ack_nack(context, true);
                } else {
                    _send_ack_nack(context, false);
                    _reset_rx_context(context);
                }
            } else if (context->received == ((size_t)context->buffer[5] + 3 + 5)) {
                size_t size = context->buffer[5] + 1;
                if (_is_data_checksum_valid(&context->buffer[5], size + 2) == true) {
                    uint32_t address = _array_to_u32(context->buffer);
                    context->io->write_mem(address, &context->buffer[6], size);
                    _send_ack_nack(context, true);
                } else {
                    _send_ack_nack(context, false);
                }
                _reset_rx_context(context);
            }
            break;

        case STM32_BOOTLOADER_CMD_GO:
            if (context->received == 5) {
                if (_is_data_checksum_valid(context->buffer, 5) == true) {
                    _send_ack_nack(context, true);
                    context->io->goto_addr(_array_to_u32(context->buffer));
                } else {
                    _send_ack_nack(context, false);
                }
                _reset_rx_context(context);
            }
            break;
        default:
            break;
    }
}

/* -------------------------------------------------------------------------- */
