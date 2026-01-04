#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "cmsis_compiler.h"
#include <string.h>

/******************************************************************************
 * common
 ******************************************************************************/
/**
 * @brief Memory placement macro
 */
#if defined(__CC_ARM)
#    define UTIL_PLACE_IN_SECTION(__x__) __attribute__((section(__x__), zero_init))
#elif defined(__ICCARM__)
#    define UTIL_PLACE_IN_SECTION(__x__) __attribute__((section(__x__)))
#else /* __GNUC__ */
#    define UTIL_PLACE_IN_SECTION(__x__) __attribute__((section(__x__)))
#endif /* __CC_ARM | __ICCARM__ | __GNUC__ */

#undef ALIGN
#ifdef WIN32
#    define ALIGN(n)
#else
#    define ALIGN(n) __attribute__((aligned(n)))
#endif /* WIN32 */

#define UTILS_INIT_CRITICAL_SECTION()
#define UTILS_ENTER_CRITICAL_SECTION()      \
    uint32_t primask_bit = __get_PRIMASK(); \
    __disable_irq()
#define UTILS_EXIT_CRITICAL_SECTION() __set_PRIMASK(primask_bit)

#ifdef __cplusplus
}
#endif
