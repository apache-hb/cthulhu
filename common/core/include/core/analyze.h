#pragma once

#include "core/compiler.h"

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
#   define RET_NOTNULL
#   define RET_STRING
#   define MUST_INSPECT

#   define FIELD_SIZE(of)
#   define FIELD_STRING
#   define FIELD_RANGE(cmp, it)

#   define IN_NOTNULL
#   define IN_STRING
#   define IN_STRING_OPT
#   define IN_RANGE(cmp, it)
#endif

#if __GNUC__ >= 11
#   define GNU_ATTRIB(...) __attribute__((__VA_ARGS__))
#else
#   define GNU_ATTRIB(...)
#endif

#ifndef NODISCARD
#   define NODISCARD GNU_ATTRIB(warn_unused_result)
#endif

#define FORMAT_ATTRIB(a, b) GNU_ATTRIB(format(printf, a, b))
#define CONSTFN GNU_ATTRIB(const)
#define PUREFN GNU_ATTRIB(pure)
#define HOTFN GNU_ATTRIB(hot)
#define COLDFN GNU_ATTRIB(cold)
#define ALLOC(...) GNU_ATTRIB(malloc, malloc(__VA_ARGS__))

#ifndef NODISCARD
#   define NODISCARD
#endif
