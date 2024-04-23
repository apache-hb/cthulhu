// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_config.h>

/// @def CT_HAS_INCLUDE(x)
/// @brief check if the compiler has a header
#if defined(__has_include)
#   define CT_HAS_INCLUDE(x) __has_include(x)
#else
#   define CT_HAS_INCLUDE(x) 0
#endif

// use _MSVC_LANG to detect c++ version on msvc
// dont want to force other projects consuming this header
// to specify /Zc:__cplusplus
#if defined(_MSVC_LANG)
#   define CT_CPLUSPLUS _MSVC_LANG
#elif defined(__cplusplus)
#   define CT_CPLUSPLUS __cplusplus
#else
#   define CT_CPLUSPLUS 0
#endif

#if CT_CPLUSPLUS >= 202002L
#   include <version>
#endif

#if __cpp_lib_unreachable >= 202202L
#   include <utility>
#endif

/// @defgroup compiler Compiler specific macros
/// @brief Compiler detection macros and compiler specific functionality
/// @ingroup core
/// @{

/// @def CT_HAS_CPP_ATTRIBUTE(x)
/// @brief check if the compiler supports a c++ attribute
#if defined(__has_cpp_attribute)
#   define CT_HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#   define CT_HAS_CPP_ATTRIBUTE(x) 0
#endif

/// @def CT_HAS_C_ATTRIBUTE(x)
/// @brief check if the compiler supports a c attribute
#if defined(__has_c_attribute)
#   define CT_HAS_C_ATTRIBUTE(x) __has_c_attribute(x)
#else
#   define CT_HAS_C_ATTRIBUTE(x) 0
#endif

/// @def CT_HAS_ATTRIBUTE(x)
/// @brief check if the compiler supports an attribute (c or c++)
#define CT_HAS_ATTRIBUTE(x) (CT_HAS_CPP_ATTRIBUTE(x) || CT_HAS_C_ATTRIBUTE(x))

/// @def CT_HAS_BUILTIN(x)
/// @brief check if the compiler has a builtin
#if defined(__has_builtin)
#   define CT_HAS_BUILTIN(x) __has_builtin(x)
#else
#   define CT_HAS_BUILTIN(x) 0
#endif

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

#if defined(_MSC_VER)
#   define CT_PRAGMA(x) __pragma(x)
#elif defined(__GNUC__)
#   define CT_PRAGMA(x) _Pragma(#x)
#else
#   define CT_PRAGMA(x)
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

#if defined(__GNUC__)
#   define CT_NORETURN_IMPL __attribute__((noreturn)) void
#elif defined(_MSC_VER)
#   define CT_NORETURN_IMPL __declspec(noreturn) void
#elif CT_HAS_ATTRIBUTE(noreturn)
#   define CT_NORETURN_IMPL [[noreturn]] void
#else
#   define CT_NORETURN_IMPL _Noreturn void
#endif

#ifdef CT_OS_WINDOWS
#   define CT_NATIVE_PATH_SEPARATOR "\\"
#   define CT_PATH_SEPERATORS "\\/"
#else
#   define CT_NATIVE_PATH_SEPARATOR "/"
#   define CT_PATH_SEPERATORS "/"
#endif

/// @def CT_UNREACHABLE()
/// @brief mark a point in code as unreachable

#if __cpp_lib_unreachable >= 202202L
#   define CT_UNREACHABLE() std::unreachable()
#elif CT_HAS_BUILTIN(__builtin_unreachable)
#   define CT_UNREACHABLE() __builtin_unreachable()
#elif defined(_MSC_VER)
#   define CT_UNREACHABLE() __assume(0)
#else
#   define CT_UNREACHABLE() ((void)0)
#endif

/// @def CT_ASSUME(expr)
/// @brief assume that @a expr is true
/// @warning this is a compiler hint that can be used to optimize code
///       use with caution

#if CT_HAS_CPP_ATTRIBUTE(assume)
#   define CT_ASSUME(expr) [[assume(expr)]]
#elif CT_HAS_BUILTIN(__builtin_assume)
#   define CT_ASSUME(expr) __builtin_assume(expr)
#elif CT_HAS_ATTRIBUTE(__assume__)
#   define CT_ASSUME(expr) __attribute__((__assume__(expr)))
#elif defined(_MSC_VER)
#   define CT_ASSUME(expr) __assume(expr)
#else
#   define CT_ASSUME(expr) do { if (!(expr)) { CT_UNREACHABLE(); } } while (0)
#endif

// clang-format off
#if CT_CPLUSPLUS
#   define CT_LINKAGE_C extern "C"
#   define CT_BEGIN_API extern "C" {
#   define CT_END_API }
#else
#   define CT_LINKAGE_C
#   define CT_BEGIN_API
#   define CT_END_API
#endif
// clang-format on

/// @def CT_FUNCTION_NAME
/// @brief the name of the current function
/// @warning the format of the string is compiler dependant, please dont try and parse it

#if defined(__PRETTY_FUNCTION__)
#   define CT_FUNCTION_NAME __PRETTY_FUNCTION__
#else
#   define CT_FUNCTION_NAME __func__
#endif

// byteswapping
#if defined(_MSC_VER) && !defined(__clang__)
#   define CT_BSWAP_U16(x) _byteswap_ushort(x)
#   define CT_BSWAP_U32(x) _byteswap_ulong(x)
#   define CT_BSWAP_U64(x) _byteswap_uint64(x)
#else
#   define CT_BSWAP_U16(x) __builtin_bswap16(x)
#   define CT_BSWAP_U32(x) __builtin_bswap32(x)
#   define CT_BSWAP_U64(x) __builtin_bswap64(x)
#endif

#if defined(_MSC_VER)
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

#if CT_CPLUSPLUS >= 202002L
#   define CT_ENUM_FLAGS(X, T) \
    constexpr X operator|(X lhs, X rhs) { return X((T)rhs | (T)lhs); } \
    constexpr X operator&(X lhs, X rhs) { return X((T)rhs & (T)lhs); } \
    constexpr X operator^(X lhs, X rhs) { return X((T)rhs ^ (T)lhs); } \
    constexpr X operator~(X rhs)        { return X(~(T)rhs); } \
    constexpr X& operator|=(X& lhs, X rhs) { return lhs = lhs | rhs; } \
    constexpr X& operator&=(X& lhs, X rhs) { return lhs = lhs & rhs; } \
    constexpr X& operator^=(X& lhs, X rhs) { return lhs = lhs ^ rhs; }
#else
#   define CT_ENUM_FLAGS(X, T)
#endif

/// @}
