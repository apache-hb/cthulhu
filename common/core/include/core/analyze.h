#pragma once

#include <ctu_config.h>

#if __cplusplus >= 201703L
#   define NODISCARD [[nodiscard]]
#endif

#if __has_include(<sal.h>)
#   include <sal.h>
#   define FORMAT_STRING _Printf_format_string_
#   define USE_DECL _Use_decl_annotations_
#   ifndef NODISCARD
#      define NODISCARD _Check_return_
#   endif
#   define IN_READS(expr) _In_reads_(expr)
#   define OUT_WRITES(expr) _Out_writes_(expr)
#   define OUT_PTR_INVALID _Post_ptr_invalid_

#   define RET_RANGE(cmp, it) _Ret_range_(cmp, it)
#   define RET_NOTNULL _Ret_notnull_
#   define RET_STRING _Ret_z_
#   define MUST_INSPECT _Must_inspect_result_

#   define FIELD_SIZE(of) _Field_size_(of)
#   define FIELD_STRING _Field_z_
#   define FIELD_RANGE(cmp, it) _Field_range_(cmp, it)

#   define IN_NOTNULL _In_
#   define IN_STRING _In_z_
#   define IN_STRING_OPT _In_opt_z_
#   define IN_RANGE(cmp, it) _In_range_(cmp, it)
#else
#   define FORMAT_STRING
#   define USE_DECL
#   define IN_READS(expr)
#   define OUT_WRITES(expr)
#   define OUT_PTR_INVALID

#   define RET_RANGE(cmp, it)
#   define RET_STRING
#   define MUST_INSPECT

#   define FIELD_STRING
#   define FIELD_RANGE(cmp, it)

#   define IN_STRING
#   define IN_STRING_OPT
#   define IN_RANGE(cmp, it)
#endif

#if __clang__ >= 10
#   define CT_PRINTF(a, b) __attribute__((__format__(__printf__, a, b)))
#elif __GNUC__ >= 11
#   define CT_PRINTF(a, b) __attribute__((format(printf, a, b)))
#else
#   define CT_PRINTF(a, b)
#endif

#if __GNUC__ >= 11
#   define GNU_ATTRIB(...) __attribute__((__VA_ARGS__))
#   define CLANG_ATTRIB(...)
#   define CTU_ATTRIB(...) __attribute__((__VA_ARGS__))
#elif __clang__ >= 10
#   define GNU_ATTRIB(...)
#   define CLANG_ATTRIB(...) __attribute__((__VA_ARGS__))
#   define CTU_ATTRIB(...) __attribute__((__VA_ARGS__))
#else
#   define CTU_ATTRIB(...)
#endif

#ifndef IN_NOTNULL
#   define IN_NOTNULL CLANG_ATTRIB(nonnull)
#endif

/// @def GNU_ATTRIB(...)
/// @brief gcc only attributes
/// @def CLANG_ATTRIB(...)
/// @brief clang only attributes
/// @def CTU_ATTRIB(...)
/// @brief any attribute that both gcc and clang support

#ifndef CLANG_ATTRIB
#   define CLANG_ATTRIB(...)
#endif

#ifndef GNU_ATTRIB
#   define GNU_ATTRIB(...)
#endif

#ifndef NODISCARD
#   define NODISCARD CTU_ATTRIB(warn_unused_result)
#endif

#if CTU_DISABLE_FN_PURITY
#   define CONSTFN
#   define PUREFN
#else
#   define CONSTFN CTU_ATTRIB(const)
#   define PUREFN CTU_ATTRIB(pure)
#endif

#define HOTFN CTU_ATTRIB(hot)
#define COLDFN CTU_ATTRIB(cold)

#define CT_ALLOC(...) CTU_ATTRIB(malloc, malloc(__VA_ARGS__))
#define CT_ALLOC_SIZE(...) CTU_ATTRIB(alloc_size(__VA_ARGS__))

#ifndef RET_NOTNULL
#   define RET_NOTNULL CTU_ATTRIB(returns_nonnull)
#endif

#ifndef FIELD_SIZE
#   define FIELD_SIZE(of) CLANG_ATTRIB(counted_by(of))
#endif

#ifndef NODISCARD
#   define NODISCARD
#endif
