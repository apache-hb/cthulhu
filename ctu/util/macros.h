#pragma once

#ifdef __GNUC__
#   define PRINT(fmt, args) __attribute__((format(printf, fmt, args)))
#   define NONULL __attribute__((nonnull))
#   define NOTNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#   define PURE __attribute__((pure))
#else
#   define PRINT(fmt, args)
#   define NONULL
#   define NOTNULL(...)
#   define PURE
#endif

#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

#define UNUSED(x) ((void)(x))

#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif
