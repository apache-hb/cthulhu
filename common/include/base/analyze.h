#pragma once

#include "base/compiler.h"

#if __has_include(<sal.h>)
#    include <sal.h>
#    define DISABLE_SAL __pragma(warning(push, 1)) __pragma(warning(disable : 6011 6240 6262 6387 28199 28278))
#    define FORMAT_STRING _Printf_format_string_
#    define USE_DECL _Use_decl_annotations_
#    define NODISCARD _Check_return_
#    define IN_READS(expr) _In_reads_(expr)
#    define OUT_WRITES(expr) _Out_writes_(expr)
#    define RET_RANGE(lo, hi) _Ret_range_(lo, hi)
#    define FIELD_SIZE(of) _Field_size_(of)
#    define MUST_INSPECT _Must_inspect_result_
#else
#    define DISABLE_SAL
#    define FORMAT_STRING
#    define USE_DECL
#    define IN_READS(expr)
#    define OUT_WRITES(expr)
#    define RET_RANGE(lo, hi)
#    define FIELD_SIZE(of)
#    define MUST_INSPECT
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
