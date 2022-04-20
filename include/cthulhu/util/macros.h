#pragma once

#ifdef __APPLE__
#   error __APPLE__
#endif

/// decorators with meaning to the compiler
#if __GNUC__ >= 11
#   define PRINT(fmt, args) __attribute__((format(printf, fmt, args)))
#   define NONULL __attribute__((nonnull))
#   define NOTNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#   define CONSTFN __attribute__((const))
#   define HOT __attribute__((hot))
#   define ALLOC(release) __attribute__((malloc(release)))
#   define DO_PRAGMA_INNER(x) _Pragma(#x)
#   define DO_PRAGMA(x) DO_PRAGMA_INNER(x)
#   define POISON(...) DO_PRAGMA(GCC poison __VA_ARGS__)
#else
#   define PRINT(fmt, args)
#   define NONULL
#   define NOTNULL(...)
#   define CONSTFN
#   define HOT
#   define ALLOC(release)
#   define POISON(...)
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

#if defined(__clang__)
#   define ASSUME(expr) __builtin_assume(expr)
#   define BEGIN_PACKED(align)
#   define END_PACKED
#   define PACKED(align) __attribute__((aligned(align), packed))
#   define NORETURN _Noreturn
#elif defined(__GNUC__)
#   define ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#   define BEGIN_PACKED(align)
#   define END_PACKED
#   define PACKED(align) __attribute__((aligned(align), packed))
#   define NORETURN _Noreturn
#elif defined(_MSC_VER)
#   define ASSUME(expr) __assume(expr)
#   define BEGIN_PACKED(align) __pragma(pack(push, align))
#   define END_PACKED __pragma(pack(pop))
#   define PACKED(align)
#   define NORETURN __declspec(noreturn)
#else
#   define ASSUME(expr)
#   define BEGIN_PACKED(align) _Pragma("warning \"current compiler doesnt support packing\"")
#   define END_PACKED
#   define PACKED(align)
#   define NORETURN
#endif

#define UNREACHABLE() ASSUME(false)

#if defined(_WIN32)
#   define CTU_WINDOWS 1
#elif defined(__linux__)
#   define CTU_LINUX 1
#else
#   error "unknown platform"
#endif

#define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)

/// macros with functionality
#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

#define ROUND2(val, mul) ((((val) + (mul) - 1) / (mul)) * (mul))


/**
 * @def ROUND2(value, multiple)
 * rounds @a value up to the nearest multiple of @a multiple
 * 
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

#define COLOUR_RED "\x1B[1;31m" ///< ANSI escape string for red
#define COLOUR_GREEN "\x1B[1;32m" ///< ANSI escape string for green
#define COLOUR_YELLOW "\x1B[1;33m" ///< ANSI escape string for yellow
#define COLOUR_BLUE "\x1B[1;34m" ///< ANSI escape string for blue
#define COLOUR_PURPLE "\x1B[1;35m" ///< ANSI escape string for purple
#define COLOUR_CYAN "\x1B[1;36m" ///< ANSI escape string for cyan
#define COLOUR_RESET "\x1B[0m" ///< ANSI escape reset

/** @} */

/// macros for headers
#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif

#ifndef _DEFAULT_SOURCE
#   define _DEFAULT_SOURCE
#   if !CTU_WINDOWS
#       include <sys/mman.h>
#   endif
#endif

#if CTU_WINDOWS
#   define ALLOCA(size) _alloca(size)
#   define STRTOK_R(str, delim, save) strtok_s(str, delim, save)
#   define STRERROR_R(err, buf, len) strerror_s(buf, len, err)
#   define PATH_LEN MAX_PATH
#else 
#   include <alloca.h>
#   define ALLOCA(size) alloca(size)
#   define STRTOK_R(str, delim, save) strtok_r(str, delim, save)
#   define STRERROR_R(err, buf, len) strerror_r(err, buf, len)
#   define PATH_LEN PATH_MAX
#endif

NORETURN PRINT(1, 2)
void ctpanic(const char *msg, ...);

#if !defined(NDEBUG) && !defined(_NDEBUG)
#   define CTASSERT(expr, msg) do { if (!(expr)) { ctpanic(COLOUR_CYAN "assert" COLOUR_RESET " [" __FILE__ ":" STR(__LINE__) "]: " msg "\n"); } } while (0)
#   define CTASSERTF(expr, msg, ...) do { if (!(expr)) { ctpanic(COLOUR_CYAN "assert" COLOUR_RESET " [" __FILE__ ":" STR(__LINE__) "]: " msg "\n", __VA_ARGS__); } } while (0)
#   define union struct
#else
#   define CTASSERT(expr, msg) do { } while (0)
#   define CTASSERTF(expr, msg, ...) do { } while (0)
#endif
