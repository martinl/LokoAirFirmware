/**
 ******************************************************************************
 * @file       compiler_abstraction.h
 * @author     DKrasutski@gmail.com
 * @version    V1.0
 * @date       22.10.2017
 * @brief      Some abstractions for popular compilers
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2018 Denis Krasutski </center></h2>
 *
 ******************************************************************************
 */
#ifndef __COMPILER_ABSTRACTION_H__
#define __COMPILER_ABSTRACTION_H__

#if defined(_MSC_VER)
#    define __packed __pragma(pack(push, 1))
#    define PACKED   __pragma(pack(pop))
#elif defined(__ICCARM__)
#    define PACKED
#elif defined(__GNUC__)
/* Include stdlib to avoid redefinition __packed macro, only for GCC build */
#    include <stdlib.h>
#    undef __packed
#    define __packed
#    define PACKED __attribute__((packed, aligned(1)))
#elif defined(__CC_ARM)
#    define PACKED
#else
#    error UNKNOWN COMPILER
#endif

#if defined(__CC_ARM) || defined(__clang__)
static inline size_t strnlen(char const *str, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (str[i] == '\x00') {
            return i;
        }
    }

    return size;
}
#endif

#endif /* __COMPILER_ABSTRACTION_H__ */
