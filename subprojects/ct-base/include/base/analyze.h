#pragma once

#include "base/compiler.h"

#if __has_include(<sal.h>)
#    include <sal.h>
#    define DISABLE_SAL __pragma(warning(push, 1)) __pragma(warning(disable : 6011 6240 6262 6387 28199 28278))
#    define FORMAT_STRING _Printf_format_string_
#    define USE_DECL _Use_decl_annotations_
#    define NODISCARD _Check_return_
#    define IN_READS(expr) _In_reads_(expr)
#    define FIELD_SIZE(of) _Field_size_(of)
#else
#    define DISABLE_SAL
#    define FORMAT_STRING
#    define USE_DECL
#    define IN_READS(expr)
#    define FIELD_SIZE(of)
#endif

#if __GNUC__ >= 11
#    define FORMAT_ATTRIBUTE(a, b) __attribute__((format(printf, a, b)))
#    define NODISCARD __attribute__((warn_unused_result))
#    define CONSTFN __attribute__((const))
#    define PUREFN __attribute__((pure))
#    define HOTFN __attribute__((hot))
#    define COLDFN __attribute__((cold))
#else
#    define FORMAT_ATTRIBUTE(a, b)
#    define CONSTFN
#    define PUREFN
#    define HOTFN
#    define COLDFN
#endif

#ifndef NODISCARD
#    define NODISCARD
#endif

#if CC_MSVC
#    define ASSUME(expr) __assume(expr)
#elif CC_CLANG || CC_GNU
#    define ASSUME(expr) __builtin_assume(expr)
#endif
