#include "CppUTest/CommandLineTestRunner.h"

#include <bsp.h>

#include "log_io.h"

int main(int ac, char **av) {
#if LOG_ENABLED == 1U
    log_init(LOG_MASK_ALL, &LOG_IO_INTERFACE);
    log_set_output_mask(LOG_MASK_OFF);
#endif  // LOG_ENABLED == 1U
    return CommandLineTestRunner::RunAllTests(ac, av);
}
