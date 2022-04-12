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
 * the BEGIN_PACKED, END_PACKED, and PACKED macros are used to pack structs
 * sadly 3 different macros are needed because msvc only uses pragma(pack)
 * and clang only uses __attribute__((packed)), hence we need to use both.
 * could we please agree on a common way to do this?
 */

#if defined(__clang__)
#   define ASSUME(expr) __builtin_assume(expr)
#   define BEGIN_PACKED(align)
#   define END_PACKED
#   define PACKED(align) __attribute__((aligned(align), packed))
#elif defined(__GNUC__)
#   define ASSUME(expr) do { if (!(expr)) __builtin_unreachable(); } while (0)
#   define BEGIN_PACKED(align)
#   define END_PACKED
#   define PACKED(align) __attribute__((aligned(align), packed))
#elif defined(_MSC_VER)
#   define ASSUME(expr) __assume(expr)
#   define BEGIN_PACKED(align) __pragma(pack(push, align))
#   define END_PACKED __pragma(pack(pop))
#   define PACKED(align)
#else
#   define ASSUME(expr)
#endif

#define UNREACHABLE() ASSUME(false)

#if defined(_WIN32)
#   define CTU_WINDOWS 1
#elif defined(__linux__)
#   define CTU_LINUX 1
#else
#   error "unknown platform"
#endif

#define NORETURN _Noreturn
#define STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)

/// macros with functionality
#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

#define ROUND2(val, mul) (((val + mul - 1) / mul) * mul)

/// macros for readability
#define UNUSED(x) ((void)(x))

#define INNER_STR(x) #x
#define STR(x) INNER_STR(x)

#define COLOUR_RED "\x1B[1;31m"
#define COLOUR_GREEN "\x1B[1;32m"
#define COLOUR_YELLOW "\x1B[1;33m"
#define COLOUR_BLUE "\x1B[1;34m"
#define COLOUR_PURPLE "\x1B[1;35m"
#define COLOUR_CYAN "\x1B[1;36m"
#define COLOUR_RESET "\x1B[0m"

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

#if __has_include(<sal.h>)
#   define IN _In_
#   define IN_OPT _In_opt_
#   define OUT _Out_
#   define OUT_OPT _Out_opt_
#   define INOUT _Inout_
#   define INOUT_OPT _Inout_opt_
#   define MAYBE _Ret_maybenull_ 
#else
#   define IN
#   define IN_OPT
#   define OUT
#   define OUT_OPT
#   define INOUT
#   define INOUT_OPT
#   define MAYBE
#endif
