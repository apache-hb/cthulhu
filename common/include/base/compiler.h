#pragma once

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
#elif defined(__APPLE__)
#    define OS_APPLE 1
#elif defined(__EMSCRIPTEN__)
#    define OS_WASM 1
#else
#    error "unknown platform"
#endif

#ifdef __cplusplus
#    define NORETURN [[noreturn]] void
#endif

#if CC_CLANG || CC_GNU
#    ifndef NORETURN
#        define NORETURN _Noreturn void
#    endif
#elif CC_MSVC
#    ifndef NORETURN
#        define NORETURN __declspec(noreturn) void
#    endif
#endif

#ifdef OS_WINDOWS
#    define NATIVE_PATH_SEPARATOR "\\"
#    define PATH_SEPERATORS "\\/"
#else
#    define NATIVE_PATH_SEPARATOR "/"
#    define PATH_SEPERATORS "/"
#endif

#ifdef CC_MSVC
#    define ASSUME(expr) __assume(expr)
#else
#    define ASSUME(expr)                                                                                               \
        do                                                                                                             \
        {                                                                                                              \
            if (!(expr))                                                                                               \
            {                                                                                                          \
                __builtin_unreachable();                                                                               \
            }                                                                                                          \
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

#if CC_GNU
#    define FUNCNAME __PRETTY_FUNCTION__
#endif

#ifndef FUNCNAME
#    define FUNCNAME __func__
#endif
