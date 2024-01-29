#pragma once

#include <ctu_config.h>

/// @defgroup analyze Static analysis decorators
/// @brief Decorators for static analysis tools
/// @ingroup core
/// @{

#if __cplusplus >= 201703L
#   define CT_NODISCARD [[nodiscard]]
#endif

#if __has_include(<sal.h>)
#   include <sal.h>
#   define CT_FMT_STRING _Printf_format_string_
#   define USE_DECL _Use_decl_annotations_
#   ifndef CT_NODISCARD
#      define CT_NODISCARD _Check_return_
#   endif
#   define IN_READS(expr) _In_reads_(expr)
#   define OUT_WRITES(expr) _Out_writes_(expr)
#   define OUT_PTR_INVALID _Post_ptr_invalid_

#   define RET_RANGE(cmp, it) _Ret_range_(cmp, it)
#   define RET_NOTNULL _Ret_notnull_
#   define RET_STRING _Ret_z_
#   define RET_INSPECT _Must_inspect_result_

#   define FIELD_SIZE(of) _Field_size_(of)
#   define FIELD_STRING _Field_z_
#   define FIELD_RANGE(cmp, it) _Field_range_(cmp, it)

#   define IN_NOTNULL _In_
#   define IN_STRING _In_z_
#   define IN_RANGE(cmp, it) _In_range_(cmp, it)
#else
#   define CT_FMT_STRING
#   define USE_DECL
#   define IN_READS(expr)
#   define OUT_WRITES(expr)
#   define OUT_PTR_INVALID

#   define RET_RANGE(cmp, it)
#   define RET_STRING
#   define RET_INSPECT

#   define FIELD_STRING
#   define FIELD_RANGE(cmp, it)

#   define IN_STRING
#   define IN_RANGE(cmp, it)
#endif

/// @def CT_PRINTF(a, b)
/// @brief mark a function as a printf style function
///
/// @param a the index of the format string parameter
/// @param b the index of the first variadic parameter


#if __clang_major__ >= 3
#   define CT_PRINTF(a, b) __attribute__((__format__(__printf__, a, b)))
#elif __GNUC__ >= 4
#   define CT_PRINTF(a, b) __attribute__((format(printf, a, b)))
#else
#  define CT_PRINTF(a, b)
#endif

#if __GNUC__ >= 11
#   define CT_GNU_ATTRIB(...) __attribute__((__VA_ARGS__))
#   define CT_CLANG_ATTRIB(...)
#   define CT_ATTRIB(...) __attribute__((__VA_ARGS__))
#elif __clang__ >= 10
#   define CT_GNU_ATTRIB(...)
#   define CT_CLANG_ATTRIB(...) __attribute__((__VA_ARGS__))
#   define CT_ATTRIB(...) __attribute__((__VA_ARGS__))
#else
#   define CT_ATTRIB(...)
#   define CT_GNU_ATTRIB(...)
#   define CT_CLANG_ATTRIB(...)
#endif

#if CT_CC_MSVC
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

#if CTU_DISABLE_FN_PURITY
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

#if CT_CC_MSVC
#   define CT_ALLOC(...) CT_DECLSPEC(restrict) CT_DECLSPEC(allocator)
#else
#   define CT_ALLOC(...) CT_ATTRIB(malloc(__VA_ARGS__))
#endif

#define CT_ALLOC_SIZE(...) CT_ATTRIB(alloc_size(__VA_ARGS__))

#ifdef __cplusplus
#   ifdef _MSC_VER
#      define CT_RESTRICT __restrict
#   elif CT_CC_GNU
#      define CT_RESTRICT __restrict__
#   elif CT_CC_CLANG
#      define CT_RESTRICT __restrict
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

#ifndef FIELD_SIZE
#   define FIELD_SIZE(of) CT_CLANG_ATTRIB(counted_by(of))
#endif

#ifndef CT_NODISCARD
#   define CT_NODISCARD
#endif

#ifdef WITH_DOXYGEN
#   define CT_NODISCARD 0
#   define RET_NOTNULL 0
#   define FIELD_SIZE(of) 0
#   define IN_NOTNULL 0
#endif

/// @def CT_NODISCARD
/// @brief mark a function as returning a value that must be used

/// @def CT_FMT_STRING
/// @brief mark a function parameter as a printf format string

/// @def USE_DECL
/// @brief sal2 annotation on function implementations to copy annotations from the declaration

/// @def IN_READS(expr)
/// @brief annotate a parameter as reading @p expr elements
///
/// @param expr the number of elements read

/// @def OUT_WRITES(expr)
/// @brief annotate a parameter as writing @p expr elements
///
/// @param expr the number of elements written

/// @def OUT_PTR_INVALID
/// @brief annotate a pointer as invalid after the function returns

/// @def RET_RANGE(cmp, it)
/// @brief annotate the return value as being bounded by the expression of @p cmp and @p it
/// RET_RANGE(!=, 0) means the returned value will never be 0
///
/// @param cmp the comparison operator
/// @param it the expression to compare against

/// @def RET_NOTNULL
/// @brief annotate the return value as not being null

/// @def RET_STRING
/// @brief annotate the return value as a null terminated string

/// @def RET_INSPECT
/// @brief annotate the return value as needing to be inspected
/// this is the same as CT_NODISCARD but implies that the return value must be checked
/// for errors

/// @def FIELD_SIZE(of)
/// @brief annotate a field as being an array of @p of elements
///
/// @param of the number of elements in the array

/// @def FIELD_STRING
/// @brief annotate a field as being a null terminated string

/// @def FIELD_RANGE(cmp, it)
/// @brief annotate a field as being bounded by the expression of @p cmp and @p it
/// FIELD_RANGE(!=, 0) means the field will never be 0
///
/// @param cmp the comparison operator
/// @param it the expression to compare against

/// @def IN_NOTNULL
/// @brief annotate a parameter as not being null

/// @def IN_STRING
/// @brief annotate a parameter as being a null terminated string

/// @def IN_RANGE(cmp, it)
/// @brief annotate a parameter as being bounded by the expression of @p cmp and @p it

/// @}
