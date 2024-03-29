// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_config.h>

/// @defgroup compiler Compiler specific macros
/// @brief Compiler detection macros and compiler specific functionality
/// @ingroup core
/// @{

// always detect clang first because it pretends to be gcc and msvc
// note: i hate that clang does this
#if defined(__clang__)
#   define CT_CC_CLANG 1
#elif defined(__GNUC__)
#   define CT_CC_GNU 1
#elif defined(_MSC_VER)
#   define CT_CC_MSVC 1
#else
#   error "unknown compiler"
#endif

#if defined(__clang__) && defined(_MSC_VER)
#   define CT_CC_CLANGCL 1
#endif

#if CT_CC_CLANG
#   define CT_CLANG_PRAGMA(x) _Pragma(#x)
#elif CT_CC_GNU
#   define CT_GNU_PRAGMA(x) _Pragma(#x)
#elif CT_CC_MSVC
#   define CT_MSVC_PRAGMA(x) __pragma(x)
#endif

#ifndef CT_CLANG_PRAGMA
#   define CT_CLANG_PRAGMA(x)
#endif

#ifndef CT_GNU_PRAGMA
#   define CT_GNU_PRAGMA(x)
#endif

#ifndef CT_MSVC_PRAGMA
#   define CT_MSVC_PRAGMA(x)
#endif

#if defined(__linux__)
#   define CT_OS_LINUX 1
#elif defined(_WIN32)
#   define CT_OS_WINDOWS 1
#elif defined(__APPLE__)
#   define CT_OS_APPLE 1
#elif defined(__EMSCRIPTEN__)
#   define CT_OS_WASM 1
#else
#   error "unknown platform"
#endif

#ifdef __cplusplus
#   define CT_NORETURN [[noreturn]] void
#endif

#ifndef CT_NORETURN
#   if CT_CC_CLANG || CT_CC_GNU
#      define CT_NORETURN __attribute__((noreturn)) void
#   elif CT_CC_MSVC
#      define CT_NORETURN __declspec(noreturn) void
#   else
#      define CT_NORETURN void
#   endif
#endif

#ifdef CT_OS_WINDOWS
#   define CT_NATIVE_PATH_SEPARATOR "\\"
#   define CT_PATH_SEPERATORS "\\/"
#else
#   define CT_NATIVE_PATH_SEPARATOR "/"
#   define CT_PATH_SEPERATORS "/"
#endif

/// @def CT_ASSUME(expr)
/// @brief assume that @a expr is true
/// @warning this is a compiler hint that can be used to optimize code
///       use with caution

#ifdef CT_CC_MSVC
#   define CT_UNREACHABLE() __assume(0)
#   define CT_ASSUME(expr) __assume(expr)
#elif CT_CC_GNU || CT_CC_CLANG
#   define CT_UNREACHABLE() __builtin_unreachable()
#   define CT_ASSUME(expr)                                                                                            \
        do                                                                                                              \
        {                                                                                                               \
            if (!(expr))                                                                                                \
            {                                                                                                           \
                __builtin_unreachable();                                                                                \
            }                                                                                                           \
        } while (0)
#else
#   define CT_UNREACHABLE() ((void)0)
#   define CT_ASSUME(expr) ((void)0)
#endif

// clang-format off
#ifdef __cplusplus
#    define CT_BEGIN_API extern "C"  {
#    define CT_END_API }
#else
#    define CT_BEGIN_API
#    define CT_END_API
#endif
// clang-format on

/// @def CT_FUNCNAME
/// @brief the name of the current function
/// @warning the format of the string is compiler dependant, please dont try and parse it

#if CT_CC_GNU && CTU_HAS_PRETTY_FUNCTION
#   define CT_FUNCNAME __PRETTY_FUNCTION__
#endif

#ifndef CT_FUNCNAME
#   define CT_FUNCNAME __func__
#endif

// byteswapping
#if CT_CC_MSVC
#   define CT_BSWAP_U16(x) _byteswap_ushort(x)
#   define CT_BSWAP_U32(x) _byteswap_ulong(x)
#   define CT_BSWAP_U64(x) _byteswap_uint64(x)
#else
#   define CT_BSWAP_U16(x) __builtin_bswap16(x)
#   define CT_BSWAP_U32(x) __builtin_bswap32(x)
#   define CT_BSWAP_U64(x) __builtin_bswap64(x)
#endif

#ifdef __cplusplus
#   define CT_LINKAGE_C extern "C"
#else
#   define CT_LINKAGE_C
#endif

// we use _MSC_VER rather than CT_CC_MSVC because both clang-cl and msvc define it
// and we want to detect both
#ifdef _MSC_VER
#   define CT_EXPORT __declspec(dllexport)
#   define CT_IMPORT __declspec(dllimport)
#   define CT_LOCAL
#elif __GNUC__ >= 4
#   define CT_EXPORT __attribute__((visibility("default")))
#   define CT_IMPORT
#   define CT_LOCAL __attribute__((visibility("internal")))
#else
#   define CT_EXPORT
#   define CT_IMPORT
#   define CT_LOCAL
#endif

#ifdef __cplusplus
namespace ctu::detail {
    template<typename T>
    struct EnumType;
}
#   define CT_ENUM_FLAGS(X) \
    template<> struct ctu::detail::EnumType<X> { using type = __underlying_type(X); }; \
    constexpr X operator|(X lhs, X rhs) { using T = ctu::detail::EnumType<X>::type; return X((T)rhs | (T)lhs); } \
    constexpr X operator&(X lhs, X rhs) { using T = ctu::detail::EnumType<X>::type; return X((T)rhs & (T)lhs); } \
    constexpr X operator^(X lhs, X rhs) { using T = ctu::detail::EnumType<X>::type; return X((T)rhs ^ (T)lhs); } \
    constexpr X operator~(X rhs)        { using T = ctu::detail::EnumType<X>::type; return X(~(T)rhs); }
#else
#   define CT_ENUM_FLAGS(X)
#endif

/// @}
