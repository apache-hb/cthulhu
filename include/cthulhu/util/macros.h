#pragma once

#ifdef __APPLE__
#    error __APPLE__
#endif

#if __has_include(<sal.h>)
#    include <sal.h>
#    define DISABLE_SAL __pragma(warning(push, 1)) __pragma(warning(disable : 6011 6240 6262 6387 28199 28278))
#    define FORMAT_STRING _Printf_format_string_
#    define USE_DECL _Use_decl_annotations_
#    define NODISCARD _Must_inspect_result_
#elif __GNUC__ >= 11
#    define FORMAT_ATTRIBUTE(a, b) __attribute__((format(printf, a, b)))
#    define NODISCARD __attribute__((warn_unused_result))
#endif

#ifndef DISABLE_SAL
#    define DISABLE_SAL
#endif

#ifndef FORMAT_STRING
#    define FORMAT_STRING
#endif

#ifndef FORMAT_ATTRIBUTE
#    define FORMAT_ATTRIBUTE(a, b)
#endif

#ifndef USE_DECL
#    define USE_DECL
#endif

#ifndef NODISCARD
#    define NODISCARD
#endif

/**
 * @defgroup Packing Struct packing macros
 * @brief cross compiler compatible struct packing macros
 *
 * the BEGIN_PACKED, END_PACKED, and PACKED macros are used to pack structs
 * sadly 3 different macros are needed because msvc only uses pragma(pack)
 * and clang only uses __attribute__((packed)), hence we need to use both.
 * could we please agree on a common way to do this?
 *
 * example usage
 * @code{.cpp}
 * #define PACKING_WIDTH 2
 * BEGIN_PACKED(PACKING_WIDTH)
 *
 * typedef struct PACKED(PACKING_WIDTH) {
 *   void *data;
 *   char field;
 * } my_packed_struct_t;
 *
 * END_PACKED
 * @endcode
 *
 * @{
 * @def BEGIN_PACKED(align) begin a struct packing area with @a align alignment
 * @def END_PACKED end a struct packing area
 * @def PACKED(align) mark a struct inside a packing area as packed to @a align
 * @}
 */

#ifdef __cplusplus
#    define NORETURN [[noreturn]] void
#endif

#if defined(__clang__)
#    define BEGIN_PACKED(align)
#    define END_PACKED()
#    define PACKED(align) __attribute__((aligned(align), packed))
#    ifndef NORETURN
#        define NORETURN _Noreturn void
#    endif
#elif defined(__GNUC__)
#    define BEGIN_PACKED(align)
#    define END_PACKED()
#    define PACKED(align) __attribute__((aligned(align), packed))
#    ifndef NORETURN
#        define NORETURN _Noreturn void
#    endif
#elif defined(_MSC_VER)
#    define BEGIN_PACKED(align) __pragma(pack(push, align))
#    define END_PACKED() __pragma(pack(pop))
#    define PACKED(align)
#    ifndef NORETURN
#        define NORETURN __declspec(noreturn) void
#    endif
#else
#    define BEGIN_PACKED(align) _Pragma("warning \"current compiler doesnt support packing\"")
#    define END_PACKED()
#    define PACKED(align)
#    ifndef NORETURN
#        define NORETURN void
#    endif
#endif

#define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)

/// macros with functionality
#define MAX(L, R) ((L) > (R) ? (L) : (R))
#define MIN(L, R) ((L) < (R) ? (L) : (R))

/**
 * @def MAX(lhs, rhs)
 * returns the maximum of @a lhs and @a rhs
 *
 * @def MIN(lhs, rhs)
 * returns the minimum of @a lhs and @a rhs
 */

/// macros for readability
#define UNUSED(x) ((void)(x))

#define INNER_STR(x) #x
#define STR(x) INNER_STR(x)

/**
 * @defgroup ColourMacros ANSI escape string colour macros
 * @brief ANSI escape string colour macros
 *
 * These are useful for formatting messages to the console.
 * @{
 */

#define COLOUR_RED "\x1B[1;31m"    ///< ANSI escape string for red
#define COLOUR_GREEN "\x1B[1;32m"  ///< ANSI escape string for green
#define COLOUR_YELLOW "\x1B[1;33m" ///< ANSI escape string for yellow
#define COLOUR_BLUE "\x1B[1;34m"   ///< ANSI escape string for blue
#define COLOUR_PURPLE "\x1B[1;35m" ///< ANSI escape string for purple
#define COLOUR_CYAN "\x1B[1;36m"   ///< ANSI escape string for cyan
#define COLOUR_RESET "\x1B[0m"     ///< ANSI escape reset

/** @} */

/// macros for headers
#ifndef _POSIX_C_SOURCE
#    define _POSIX_C_SOURCE 200112L
#endif

#ifndef _DEFAULT_SOURCE
#    define _DEFAULT_SOURCE
#    ifndef _WIN32
#        include <sys/mman.h>
#    endif
#endif

NORETURN ctpanic(FORMAT_STRING const char *msg, ...) FORMAT_ATTRIBUTE(1, 2);

#if ENABLE_DEBUG
#    define CTASSERT(expr, msg)                                                                                        \
        do                                                                                                             \
        {                                                                                                              \
            if (!(expr))                                                                                               \
            {                                                                                                          \
                ctpanic(COLOUR_CYAN "assert" COLOUR_RESET " [" __FILE__ ":" STR(__LINE__) "]: " msg "\n");             \
            }                                                                                                          \
        } while (0)
#    define CTASSERTF(expr, msg, ...)                                                                                  \
        do                                                                                                             \
        {                                                                                                              \
            if (!(expr))                                                                                               \
            {                                                                                                          \
                ctpanic(COLOUR_CYAN "assert" COLOUR_RESET " [" __FILE__ ":" STR(__LINE__) "]: " msg "\n",              \
                        __VA_ARGS__);                                                                                  \
            }                                                                                                          \
        } while (0)
#    define union struct
#else
#    define CTASSERT(expr, msg)                                                                                        \
        do                                                                                                             \
        {                                                                                                              \
        } while (0)
#    define CTASSERTF(expr, msg, ...)                                                                                  \
        do                                                                                                             \
        {                                                                                                              \
        } while (0)
#endif
