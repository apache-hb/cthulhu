#pragma once

/// decorators with meaning to the compiler
#if __GNUC__ >= 11
#   define PRINT(fmt, args) __attribute__((format(printf, fmt, args)))
#   define NONULL __attribute__((nonnull))
#   define NOTNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#   define CONSTFN __attribute__((const))
#   define HOT __attribute__((hot))
#   define ALLOC(release) __attribute__((malloc(release)))
#   define POISON(...) _Pragma GCC poison __VA_ARGS__
#else
#   define PRINT(fmt, args)
#   define NONULL
#   define NOTNULL(...)
#   define CONSTFN
#   define HOT
#   define ALLOC(release)
#   define POISON(...)
#endif

#if defined(_WIN32)
#   define CTU_WINDOWS 1
#elif defined(__linux__)
#   define CTU_LINUX 1
#else
#   error "unknown platform"
#endif

#define NORETURN _Noreturn

/// macros with functionality
#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

/// macros for readability
#define UNUSED(x) ((void)(x))

#define INNER_STR(x) #x
#define STR(x) INNER_STR(x)

#define WEAK /// this pointer does not own its data
#define OWNED /// this pointer owns its data
#define MOVE /// moves ownership of data
#define NULLABLE /// pointer can be null

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
#   define PATH_LEN MAX_PATH
#else 
#   define ALLOCA(size) alloca(size)
#   define STRTOK_R(str, delim, save) strtok_r(str, delim, save)
#   define PATH_LEN PATH_MAX
#endif

NORETURN PRINT(1, 2)
void ctpanic(const char *msg, ...);

#define CTASSERT(expr, msg) if (!(expr)) { ctpanic(COLOUR_CYAN "assert" COLOUR_RESET " [" STR(__FILE__) ":" STR(__LINE__) "]: " msg "\n"); }
#define CTASSERTF(expr, msg, ...) if (!(expr)) { ctpanic(COLOUR_CYAN "assert" COLOUR_RESET " [" STR(__FILE__) ":" STR(__LINE__) "]: " msg "\n", __VA_ARGS__); }
