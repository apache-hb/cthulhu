// SPDX-License-Identifier: LGPL-3.0-or-later
#pragma once

// use _MSVC_LANG to detect the c++ version on msvc
// otherwise we would force other projects consuming this header
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

/// @def CT_HAS_CXX_ATTRIBUTE(x)
/// @brief check if the compiler supports a c++ attribute
#if defined(__has_cpp_attribute)
#   define CT_HAS_CXX_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#   define CT_HAS_CXX_ATTRIBUTE(x) 0
#endif

#if CT_CPLUSPLUS >= 202002L
#   define CTX_TEST(name, ...) static_assert([] { __VA_ARGS__ }(), name)
#   define CTX_CONSTEXPR constexpr
#   define CTX_IMPL(...) __VA_ARGS__
#else
#   define CTX_TEST(name)
#   define CTX_CONSTEXPR
#   define CTX_IMPL(...)
#endif
