#pragma once

// not worth the hassle
#ifdef __APPLE__
#    error __APPLE__
#endif

#if defined(__clang__)
#    define CC_CLANG 1
#elif defined(__GNUC__)
#    define CC_GNU 1
#elif defined(_MSC_VER)
#    define CC_MSVC 1
#else
#    error "unknown compiler"
#endif

#if defined(__linux__)
#    define OS_LINUX 1
#elif defined(_WIN32)
#    define OS_WINDOWS 1
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

#if CC_CLANG || CC_GNU
#    define BEGIN_PACKED(align)
#    define END_PACKED()
#    define PACKED(align) __attribute__((aligned(align), packed))
#    ifndef NORETURN
#        define NORETURN _Noreturn void
#    endif
#elif CC_MSVC
#    define BEGIN_PACKED(align) __pragma(pack(push, align))
#    define END_PACKED() __pragma(pack(pop))
#    define PACKED(align)
#    ifndef NORETURN
#        define NORETURN __declspec(noreturn) void
#    endif
#endif

#ifdef OS_WINDOWS
#    define PATH_SEP "\\"
#else
#    define PATH_SEP "/"
#endif
