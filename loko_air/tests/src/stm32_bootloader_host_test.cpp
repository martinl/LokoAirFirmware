#include "CppUTest/TestHarness.h"

#include "stm32_bootloader_host_protocol.h"
#include <string.h>
#include <vector>

std::vector<uint8_t> response(256);

#define _SEND(array) bootloader_send(array, sizeof(array) - 1)

#if DEBUG_BUILD == 1
#    define _LOG printf
#else
#    define _LOG(...)
#endif

TEST_GROUP(stm32_bootloader_host_test_exchange) {

    stm32_bootloader_context_t context;
    stm32_bootloader_io_t io = { serial_out, read_mem, write_mem, erase_page, goto_addr };
    void setup() final {
        response.clear();
        CHECK_EQUAL(STM32_BOOTLOADER_RESULT_OK, stm32_bootloader_host_protocol_init(&context, &io));
    }

    void teardown() final {
        CHECK_EQUAL(STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND, context.state);
        CHECK_EQUAL(0, context.received);
    }

    static void goto_addr(size_t addr) {
        (void)addr;
        _LOG("Go to addr: 0x%08X\n", addr);
    }

    static void erase_page(size_t page_number) {
        (void)page_number;
        _LOG("Erase page: %d\n", page_number);
    }

    static void write_mem(size_t addr, uint8_t const *byte, size_t size) {
        (void)addr;
        (void)byte;
        (void)size;

        _LOG("Write mem: 0x%08X, Size %d\n", addr, size);
        for (size_t i = 0; i < size; i++) {
            _LOG("%02X ", byte[i]);
        }
        _LOG("\n");
    }

    static void read_mem(size_t addr, uint8_t * byte, size_t size) {
        (void)addr;
        (void)byte;
        (void)size;
        for (size_t i = 0; i < size; i++) {
            byte[i] = (uint8_t)i;
        }
        _LOG("Read mem: 0x%08X, Size %d\n", addr, size);
    }

    static void serial_out(uint8_t const *byte, size_t size) {
        _LOG("\rRX: ");
        for (size_t i = 0; i < size; i++) {
            response.push_back(byte[i]);
            _LOG("%02X ", byte[i]);
        }
        _LOG("\n");
    }

    void bootloader_send(char const *bytes, size_t size) {
        response.clear();

        _LOG("\rTX: ");
        for (size_t i = 0; i < size; i++) {
            _LOG("%02X ", (uint8_t)bytes[i]);
        }
        _LOG("\n");

        for (size_t i = 0; i < size; i++) {
            stm32_bootloader_host_protocol_byte_handle(&context, (uint8_t)bytes[i]);
        }
    }

    bool check_response(char const expected[]) const {
        return std::equal(response.begin(), response.end(), (uint8_t const *)expected);
    }
};

TEST(stm32_bootloader_host_test_exchange, goto_08008000_command) {
    _SEND("\x21\xde");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x80\x00\x88");
    CHECK(check_response("\x79"));
}

TEST(stm32_bootloader_host_test_exchange, goto_command_wrong_checksum) {
    _SEND("\x21\xde");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x80\x00\x00");
    CHECK(check_response("\x1F"));
}

TEST(stm32_bootloader_host_test_exchange, write_4byte_command) {
    _SEND("\x31\xce");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x80\x00\x88");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK(check_response("\x79"));
    _SEND("\x03\x10\x20\x40\x80\xF3");
    CHECK(check_response("\x79"));
}

TEST(stm32_bootloader_host_test_exchange, write_4byte_command_wrong_addr_checksum) {
    _SEND("\x31\xce");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x80\x00\x11");
    CHECK(check_response("\x1F"));
}

TEST(stm32_bootloader_host_test_exchange, write_4byte_command_wrong_data_checksum) {
    _SEND("\x31\xce");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x80\x00\x88");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK(check_response("\x79"));
    _SEND("\x03\x10\x20\x40\x80\x03");
    CHECK(check_response("\x1F"));
}

TEST(stm32_bootloader_host_test_exchange, write_1byte_command) {
    _SEND("\x31\xce");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x80\x00\x88");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK(check_response("\x79"));
    _SEND("\x00\x00\x00");
    CHECK(check_response("\x79"));
}

TEST(stm32_bootloader_host_test_exchange, erase_page_n_command) {
    _SEND("\x43\xbc");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x01\x10\x01\x10");
    CHECK(check_response("\x79"));
}

TEST(stm32_bootloader_host_test_exchange, erase_page_n_command_bad_checksum) {
    _SEND("\x43\xbc");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x01\x10\x01\xFF");
    CHECK(check_response("\x1F"));
}

TEST(stm32_bootloader_host_test_exchange, erase_global_command) {
    _SEND("\x43\xbc");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\xFF\x00");
    CHECK(check_response("\x79"));
}

TEST(stm32_bootloader_host_test_exchange, get_addr_1fff73f8_len3_command) {
    _SEND("\x11\xEE");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x1F\xFF\x73\xF8\x6B");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK(check_response("\x79"));
    _SEND("\x03\xFC");
    CHECK(check_response("\x79\x00\x01\x02\x03"));
}

TEST(stm32_bootloader_host_test_exchange, get_addr_len16_command) {
    _SEND("\x11\xEE");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x00\x00\x08");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK(check_response("\x79"));
    _SEND("\x10\xEF");
    CHECK(check_response("\x79\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x0F\x10"));
}

TEST(stm32_bootloader_host_test_exchange, get_addr_len0_command) {
    _SEND("\x11\xEE");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x00\x00\x08");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK(check_response("\x79"));
    _SEND("\x00\xFF");
    CHECK(check_response("\x79"));
}

TEST(stm32_bootloader_host_test_exchange, get_addr_len0_command_wrong_checksum) {
    _SEND("\x11\xEE");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x00\x00\x11");  // Wrong checksum
    CHECK(check_response("\x1F"));
}

TEST(stm32_bootloader_host_test_exchange, get_addr_len0_command_wrong_checksum_2) {
    _SEND("\x11\xEE");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x79"));
    _SEND("\x08\x00\x00\x00\x08");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_COMMAND_PROCCESS, context.state);
    CHECK(check_response("\x79"));
    _SEND("\x00\xF0");  // Wrong checksum
    CHECK(check_response("\x1F"));
}

TEST(stm32_bootloader_host_test_exchange, get_checksum_command_unsupported) {
    _SEND("\xA1\x5E");
    CHECK(check_response("\x1F"));
}

TEST(stm32_bootloader_host_test_exchange, get_version_command_ok) {
    _SEND("\x01\xFE");
    CHECK(check_response("\x79\x10\x00\x00\x79"));
}

TEST(stm32_bootloader_host_test_exchange, get_command_ok) {
    _SEND("\x00\xFF");
    CHECK(check_response("\x79\x06\x10\x00\x01\x02\x011\x21\x31\x43\x79"));
}

TEST(stm32_bootloader_host_test_exchange, get_id_command_wrong_checksum) {
    _SEND("\x02\xF0");
    CHECK(check_response("\x1F"));

    _SEND("\x02\x02");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x1F"));
}

TEST(stm32_bootloader_host_test_exchange, get_id_command_ok) {
    _SEND("\x02\xFD");
    CHECK(check_response("\x79\x01\x04\x97\x79"));
}

TEST(stm32_bootloader_host_test_exchange, ff_spam) {
    for (size_t i = 0; i < 1024; i++) {
        _SEND("\xFF");
    }
}

TEST(stm32_bootloader_host_test_exchange, non_command_init_command) {

    _SEND("\xFF");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND, context.state);
    CHECK_EQUAL(1, context.received);

    _SEND("\x7F");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND, context.state);
    CHECK_EQUAL(0, context.received);
    CHECK(check_response("\x1F"));

    _SEND("\x7F");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND, context.state);
    CHECK_EQUAL(0, context.received);

    CHECK(check_response("\x79"));
}

TEST(stm32_bootloader_host_test_exchange, non_command) {

    _SEND("\xFF");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND, context.state);
    CHECK_EQUAL(1, context.received);

    _SEND("\xFF");
    CHECK_EQUAL(STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND, context.state);
    CHECK_EQUAL(0, context.received);
}

TEST(stm32_bootloader_host_test_exchange, init_command) {

    _SEND("\x7F");

    CHECK_EQUAL(STM32_BOOTLOADER_STATE_WAIT_FOR_COMMAND, context.state);
    CHECK_EQUAL(0, context.received);

    CHECK(check_response("\x79"));
}

TEST_GROUP(stm32_bootloader_host_test_init) {

    void setup() final {
        //
    }

    void teardown() final {
        //
    }

    stm32_bootloader_io_t io;

    static void erase_page(size_t page_number) {
        (void)page_number;
    }

    static void write_mem(size_t addr, uint8_t const *byte, size_t size) {
        (void)addr;
        (void)byte;
        (void)size;
    }

    static void read_mem(size_t addr, uint8_t * byte, size_t size) {
        (void)addr;
        (void)byte;
        (void)size;
    }

    static void serial_out(uint8_t const *byte, size_t size) {
        (void)byte;
        (void)size;
    }

    static void goto_addr(size_t addr) {
        (void)addr;
    }
};

TEST(stm32_bootloader_host_test_init, init_args) {
    stm32_bootloader_context_t context;
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_ERROR, stm32_bootloader_host_protocol_init(0, 0));
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_ERROR, stm32_bootloader_host_protocol_init(&context, 0));
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_ERROR, stm32_bootloader_host_protocol_init(0, &io));
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_ERROR, stm32_bootloader_host_protocol_init(&context, &io));

    io.erase_page = erase_page;
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_ERROR, stm32_bootloader_host_protocol_init(&context, &io));
    io.read_mem = read_mem;
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_ERROR, stm32_bootloader_host_protocol_init(&context, &io));
    io.serial_out = serial_out;
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_ERROR, stm32_bootloader_host_protocol_init(&context, &io));
    io.write_mem = write_mem;
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_ERROR, stm32_bootloader_host_protocol_init(&context, &io));
    io.goto_addr = goto_addr;
    CHECK_EQUAL(STM32_BOOTLOADER_RESULT_OK, stm32_bootloader_host_protocol_init(&context, &io));
}
