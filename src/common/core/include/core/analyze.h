// SPDX-License-Identifier: LGPL-3.0-only

#pragma once

#include <ctu_config.h>

#include "core/compiler.h"

/// @defgroup analyze Static analysis decorators
/// @brief Decorators for static analysis tools
/// @ingroup core
/// @{

///
/// static analysis macros
/// old macros not prefixed with STA_ are deprecated
///

// work around gcc bug where attributes are reported as available in C11 mode
// but are not actually available.
#if CT_HAS_ATTRIBUTE(nodiscard) && CT_CPLUSPLUS && !defined(__GNUC__)
#   define CT_NODISCARD [[nodiscard]]
#endif

#if defined(_PREFAST_)
#   include <sal.h>
#   ifndef CT_NODISCARD
#      define CT_NODISCARD _Check_return_
#   endif

#   define RET_DOMAIN(cmp, it) _Ret_range_(cmp, it)
#   define RET_NOTNULL _Ret_notnull_
#   define RET_INSPECT _Must_inspect_result_

#   define IN_NOTNULL _In_
#   define IN_STRING _In_z_
#   define IN_DOMAIN(cmp, it) _In_range_(cmp, it)

#   define OUT_NOTNULL _Out_

#   define INOUT_NOTNULL _Inout_

#   define STA_PRESENT 1

    // other annotations
#   define STA_DECL _Use_decl_annotations_

    // return value annotations
#   define STA_RET_NEVER _Analysis_noreturn_
#   define STA_RET_RANGE(lo, hi) _Ret_range_(lo, hi)
#   define STA_RET_NOTNULL _Ret_notnull_
#   define STA_RET_STRING _Ret_z_

    // success/failure annotations
#   define STA_SUCCESS(expr) _Success_(expr)
#   define STA_SUCCESS_TYPE(expr) _Return_type_success_(expr)
#   define STA_LAST_ERROR _Post_equals_last_error_

    // struct annotations
#   define STA_FIELD_SIZE(of) _Field_size_(of)
#   define STA_FIELD_STRING _Field_z_
#   define STA_FIELD_RANGE(lo, hi) _Field_range_(lo, hi)

    // array parameter annotations
#   define STA_UPDATES(size) _Inout_updates_(size)
#   define STA_READS(size) _In_reads_(size)
#   define STA_WRITES(size) _Out_writes_(size)

    // string parameter annotations
#   define STA_UPDATES_CSTRING(size) _Inout_updates_z_(size)
#   define STA_READS_CSTRING(size) _In_reads_z_(size)
#   define STA_WRITES_CSTRING(size) _Out_writes_z_(size)

    // format string parameter annotations
#   define STA_FORMAT_STRING _Printf_format_string_

    // out parameter annotations
#   define STA_OUT_OPT _Out_opt_
#   define STA_OUT_CSTRING _Out_z_
#   define STA_OUT_RANGE(lo, hi) _Out_range_(lo, hi)
#   define STA_OUT_INVALID _Post_ptr_invalid_

    // in parameter annotations
#   define STA_IN_RANGE(lo, hi) _In_range_(lo, hi)

    // inout parameter annotations
#   define STA_INOUT _Inout_
#   define STA_INOUT_OPT _Inout_opt_
#   define STA_INOUT_CSTRING _Inout_z_
#else
#   define RET_DOMAIN(cmp, it)
#   define RET_NOTNULL
#   define RET_INSPECT

#   define IN_NOTNULL
#   define IN_STRING
#   define IN_DOMAIN(cmp, it)

#   define OUT_NOTNULL

#   define INOUT_NOTNULL

    // other annotations
#   define STA_DECL

    // return value annotations
#   define STA_RET_NEVER
#   define STA_RET_RANGE(lo, hi)
#   define STA_RET_NOTNULL
#   define STA_RET_STRING

    // success/failure annotations
#   define STA_SUCCESS(expr)
#   define STA_SUCCESS_TYPE(expr)
#   define STA_LAST_ERROR

    // struct annotations
#   define STA_FIELD_SIZE(of)
#   define STA_FIELD_STRING
#   define STA_FIELD_RANGE(lo, hi)

    // array parameter annotations
#   define STA_UPDATES(size)
#   define STA_READS(size)
#   define STA_WRITES(size)

    // string parameter annotations
#   define STA_UPDATES_CSTRING(size)
#   define STA_READS_CSTRING(size)
#   define STA_WRITES_CSTRING(size)

    // format string parameter annotations
#   define STA_FORMAT_STRING

    // out parameter annotations
#   define STA_OUT_OPT
#   define STA_OUT_CSTRING
#   define STA_OUT_RANGE(lo, hi)
#   define STA_OUT_INVALID

    // in parameter annotations
#   define STA_IN_RANGE(lo, hi)

    // inout parameter annotations
#   define STA_INOUT
#   define STA_INOUT_OPT
#   define STA_INOUT_CSTRING
#endif

#define CT_NORETURN STA_RET_NEVER CT_NORETURN_IMPL

#define STA_RELEASE IN_NOTNULL STA_OUT_INVALID

/// @def STA_PRINTF(a, b)
/// @brief mark a function as a printf style function
///
/// @param a the index of the format string parameter
/// @param b the index of the first variadic parameter

#if __clang_major__ >= 3
#   define STA_PRINTF(a, b) __attribute__((__format__(__printf__, a, b)))
#elif __GNUC__ >= 4
#   define STA_PRINTF(a, b) __attribute__((format(printf, a, b)))
#else
#  define STA_PRINTF(a, b)
#endif

#if __GNUC__ >= 11
#   define CT_GNU_ATTRIB(...) __attribute__((__VA_ARGS__))
#   define CT_CLANG_ATTRIB(...)
#   define CT_ATTRIB(...) __attribute__((__VA_ARGS__))
#elif __clang_major__ >= 10
#   define CT_GNU_ATTRIB(...)
#   define CT_CLANG_ATTRIB(...) __attribute__((__VA_ARGS__))
#   define CT_ATTRIB(...) __attribute__((__VA_ARGS__))
#else
#   define CT_ATTRIB(...)
#   define CT_GNU_ATTRIB(...)
#   define CT_CLANG_ATTRIB(...)
#endif

#if defined(_MSC_VER)
#   define CT_DECLSPEC(...) __declspec(__VA_ARGS__)
#else
#   define CT_DECLSPEC(...)
#endif

#ifndef IN_NOTNULL
#   define IN_NOTNULL CT_CLANG_ATTRIB(nonnull)
#endif

/// @def CT_GNU_ATTRIB(...)
/// @brief gcc only attributes
/// @def CT_CLANG_ATTRIB(...)
/// @brief clang only attributes
/// @def CT_ATTRIB(...)
/// @brief any attribute that both gcc and clang support

#ifndef CT_NODISCARD
#   define CT_NODISCARD CT_ATTRIB(warn_unused_result)
#endif

/// @def CT_NOALIAS
/// @brief mark a function as only modifying pointers passed to it
/// the same as @a CT_CONSTFN but allowed to modify/inspect pointers passed to it
///
/// @def CT_CONSTFN
/// @brief mark a function as const, has no side effects and always returns the same value for the same arguments
/// @warning do not apply this to functions that take pointers as arguments
///
/// @def CT_PUREFN
/// @brief mark a function as pure, always returns the same value for the same arguments
/// @warning must not depend on mutable global state or have side effects
/// @note thats a lie actually, gcc says it may have calls to it optimized away via CSE

// TODO: this is a bit of a hack to make tests work properly
// both gcc and clang will optimize away calls to pure/const functions
// if they can statically prove that the function will cause an assertion.
// In our test suite we test assertion cases so we need to have a way to disable
// this optimization. This may technically be UB.
#if CT_DISABLE_FN_PURITY
#   define CT_NOALIAS
#   define CT_CONSTFN
#   define CT_PUREFN
#else
#   define CT_NOALIAS CT_DECLSPEC(noalias)
#   define CT_CONSTFN CT_ATTRIB(const)
#   define CT_PUREFN CT_ATTRIB(pure)
#endif

/// @def CT_ALLOC
/// @brief mark a function as allocating memory
/// @def CT_ALLOC_SIZE
/// @brief mark a function as allocating memory with a specific size

// clang doesnt support passing a deallocate function to attribute((malloc))
#if defined(__clang__)
#   define CT_ALLOC(...) __attribute__((malloc))
#elif defined(__GNUC__)
#   define CT_ALLOC(...) __attribute__((malloc(__VA_ARGS__)))
#elif defined(_MSC_VER)
#   define CT_ALLOC(...) __declspec(restrict) __declspec(allocator)
#else
#   define CT_ALLOC(...)
#endif

#define CT_ALLOC_SIZE(...) CT_ATTRIB(alloc_size(__VA_ARGS__))

#if CT_CPLUSPLUS
#   if defined(_MSC_VER)
#      define CT_RESTRICT __restrict
#   elif defined(__clang__)
#      define CT_RESTRICT __restrict
#   elif defined(__GNUC__)
#      define CT_RESTRICT __restrict__
#   else
#      define CT_RESTRICT
#   endif
#else
#   define CT_RESTRICT restrict
#endif

/// @def CT_HOTFN
/// @brief mark a function as hot, it is likely to be called often
/// @def CT_COLDFN
/// @brief mark a function as cold, it is unlikely to be called often

#define CT_HOTFN CT_ATTRIB(hot)
#define CT_COLDFN CT_ATTRIB(cold)

#ifndef RET_NOTNULL
#   define RET_NOTNULL CT_ATTRIB(returns_nonnull)
#endif

#if !defined(STA_FIELD_SIZE) && CT_HAS_ATTRIBUTE(counted_by)
#   define STA_FIELD_SIZE(of) __attribute__((counted_by(of)))
#endif

#ifndef CT_NODISCARD
#   define CT_NODISCARD
#endif

#ifdef WITH_DOXYGEN
#   define CT_NODISCARD 0
#   define RET_NOTNULL 0
#   define IN_NOTNULL 0
#endif

/// @def CT_NODISCARD
/// @brief mark a function as returning a value that must be used

/// @def STA_FORMAT_STRING
/// @brief mark a function parameter as a printf format string

/// @def STA_DECL
/// @brief sal2 annotation on function implementations to copy annotations from the declaration

/// @def STA_READS(expr)
/// @brief annotate a parameter as reading @p expr elements
///
/// @param expr the number of elements read

/// @def STA_WRITES(expr)
/// @brief annotate a parameter as writing @p expr elements
///
/// @param expr the number of elements written

/// @def STA_RELEASE
/// @brief annotate a pointer as invalid after the function returns

/// @def RET_DOMAIN(cmp, it)
/// @brief annotate the return value as being bounded by the expression of @p cmp and @p it
/// RET_DOMAIN(!=, 0) means the returned value will never be 0
///
/// @param cmp the comparison operator
/// @param it the expression to compare against

/// @def RET_NOTNULL
/// @brief annotate the return value as not being null

/// @def STA_RET_STRING
/// @brief annotate the return value as a null terminated string

/// @def RET_INSPECT
/// @brief annotate the return value as needing to be inspected
/// this is the same as CT_NODISCARD but implies that the return value must be checked
/// for errors

/// @def STA_FIELD_SIZE(of)
/// @brief annotate a field as being an array of @p of elements
///
/// @param of the number of elements in the array

/// @def STA_FIELD_STRING
/// @brief annotate a field as being a null terminated string

/// @def STA_FIELD_RANGE(cmp, it)
/// @brief annotate a field as being bounded by the expression of @p cmp and @p it
/// STA_FIELD_RANGE(!=, 0) means the field will never be 0
///
/// @param cmp the comparison operator
/// @param it the expression to compare against

/// @def IN_NOTNULL
/// @brief annotate a parameter as not being null

/// @def IN_STRING
/// @brief annotate a parameter as being a null terminated string

/// @def IN_DOMAIN(cmp, it)
/// @brief annotate a parameter as being bounded by the expression of @p cmp and @p it

/// @}
