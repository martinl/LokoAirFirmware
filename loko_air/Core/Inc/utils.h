#pragma once

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stdint.h>

#ifndef MIN
#    define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif  // MIN

#ifndef MAX
#    define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif  // MAX

#ifndef COUNT_OF
#    define COUNT_OF(x) ((sizeof(x) / sizeof(0 [x])) / ((size_t)(!(sizeof(x) % sizeof(0 [x])))))
#endif  // MAX

#define COMPILE_TIME_ASSERT(pred) \
    switch (0) {                  \
        case 0:                   \
        case pred:;               \
    }

#if (USE_ASSERT == 1U)
#    define ASSERT(cond)                                       \
        do                                                     \
            if (!(cond)) {                                     \
                LOG_ERROR("Assert:%s:%d", __FILE__, __LINE__); \
                bsp_disable_irq();                             \
                for (;;) {}                                    \
            }                                                  \
        while (0)
#else
#    define ASSERT(cond) \
        do {             \
        } while (0)
#endif

#define __DEF_TO_STR(def) #def
#define DEF_TO_STR(def)   __DEF_TO_STR(def)

/* Static ASSERT implementation with unique object name */
#define _ST_CONCAT(x, y)                 x##y
#define _ST_ASSERT_IMPL(condition, line) extern char _ST_CONCAT(__static_assert_line__, line)[(condition) ? 1 : -1]
#define STATIC_ASSERT(condition)         _ST_ASSERT_IMPL((condition), __LINE__)

#ifndef UNUSED
#    define UNUSED(var) ((void)var)
#endif  // UNUSED

#ifndef IS_ODD
#    define IS_ODD(var) ((var)&1)
#endif  // IS_ODD

#define INTER_TARGET_MAILBOX_CMD_STAY_IN_BOOTLOADER 0x01020304UL
#define INTER_TARGET_MAILBOX_SEND(message)   \
    do {                                     \
        bsp_rtc_store_write_reg(0, message); \
    } while (0)

#define INTER_TARGET_MAILBOX_GET() bsp_rtc_store_read_reg(0)

#ifdef __cplusplus
}
#endif /* __cplusplus */
