#pragma once

#include "base/compiler.h"

#if __cplusplus >= 201703L
#   define NODISCARD [[nodiscard]]
#endif

#if __has_include(<sal.h>)
#    include <sal.h>
#    define FORMAT_STRING _Printf_format_string_
#    define USE_DECL _Use_decl_annotations_
#    ifndef NODISCARD
#       define NODISCARD _Check_return_
#    endif
#    define IN_READS(expr) _In_reads_(expr)
#    define OUT_WRITES(expr) _Out_writes_(expr)
#    define RET_RANGE(lo, hi) _Ret_range_(lo, hi)
#    define FIELD_SIZE(of) _Field_size_(of)
#    define MUST_INSPECT _Must_inspect_result_
#    define IN_NOTNULL _In_
#    define IN_NULLABLE _In_opt_
#    define IN_STRING _In_z_
#    define IN_RANGE(lo, hi) _In_range_(lo, hi)
#else
#    define FORMAT_STRING
#    define USE_DECL
#    define IN_READS(expr)
#    define OUT_WRITES(expr)
#    define RET_RANGE(lo, hi)
#    define FIELD_SIZE(of)
#    define MUST_INSPECT
#    define IN_NOTNULL
#    define IN_NULLABLE
#    define IN_STRING
#    define IN_RANGE(lo, hi)
#endif

#if __GNUC__ >= 11
#    define GNU_ATTRIB(...) __attribute__(( __VA_ARGS__ ))
#else
#    define GNU_ATTRIB(...)
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
#    define NODISCARD
#endif
