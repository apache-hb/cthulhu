#pragma once

#include <ctu_config.h>

/// @defgroup Compiler Compiler specific macros
/// @brief Compiler detection macros and compiler specific functionality
/// @ingroup Core
/// @{

#if defined(__clang__)
#   define CC_CLANG 1
#elif defined(__GNUC__)
#   define CC_GNU 1
#elif defined(_MSC_VER)
#   define CC_MSVC 1
#else
#   error "unknown compiler"
#endif

#if defined(__linux__)
#   define OS_LINUX 1
#elif defined(_WIN32)
#   define OS_WINDOWS 1
#elif defined(__APPLE__)
#   define OS_APPLE 1
#elif defined(__EMSCRIPTEN__)
#   define OS_WASM 1
#else
#   error "unknown platform"
#endif

#ifdef __cplusplus
#   define NORETURN [[noreturn]] void
#endif

#ifndef NORETURN
#   if CC_CLANG || CC_GNU
#      define NORETURN __attribute__((noreturn)) void
#   elif CC_MSVC
#      define NORETURN __declspec(noreturn) void
#   else
#      define NORETURN void
#   endif
#endif

#ifdef OS_WINDOWS
#   define NATIVE_PATH_SEPARATOR "\\"
#   define PATH_SEPERATORS "\\/"
#else
#   define NATIVE_PATH_SEPARATOR "/"
#   define PATH_SEPERATORS "/"
#endif


/// @def CTU_ASSUME(expr)
/// @brief assume that @a expr is true
/// @warning this is a compiler hint that can be used to optimize code
///       use with caution

#ifdef CC_MSVC
#   define CTU_ASSUME(expr) __assume(expr)
#else
#   define CTU_ASSUME(expr)                                                                                            \
        do                                                                                                              \
        {                                                                                                               \
            if (!(expr))                                                                                                \
            {                                                                                                           \
                __builtin_unreachable();                                                                                \
            }                                                                                                           \
        } while (0)
#endif

// clang-format off
#ifdef __cplusplus
#    define BEGIN_API extern "C"  {
#    define END_API }
#else
#    define BEGIN_API
#    define END_API
#endif
// clang-format on

/// @def FUNCNAME
/// @brief the name of the current function
/// @warning the format of the string is compiler dependant, please dont try and parse it

#if CC_GNU && CTU_HAS_PRETTY_FUNCTION
#   define FUNCNAME __PRETTY_FUNCTION__
#endif

#ifndef FUNCNAME
#   define FUNCNAME __func__
#endif

/// @}
