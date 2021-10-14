#pragma once

/// decorators with meaning to the compiler
#if __GNUC__ >= 11
#   define PRINT(fmt, args) __attribute__((format(printf, fmt, args)))
#   define NONULL __attribute__((nonnull))
#   define NOTNULL(...) __attribute__((nonnull(__VA_ARGS__)))
#   define CONSTFN __attribute__((const))
#   define HOT __attribute__((hot))
#   define ALLOC(release) __attribute__((malloc(release)))
#   define POISON(...) _Pragma GCC poison __VA_ARGS__
#else
#   define PRINT(fmt, args)
#   define NONULL
#   define NOTNULL(...)
#   define CONSTFN
#   define HOT
#   define ALLOC(release)
#   define POISON(...)
#endif

/// macros with functionality
#define MAX(L, R) ((L) > (R) ? (L) : (R)) 
#define MIN(L, R) ((L) < (R) ? (L) : (R)) 

/// macros for readability
#define UNUSED(x) ((void)(x))

#define WEAK /// this pointer does not own its data
#define OWNED /// this pointer owns its data
#define MOVE /// moves ownership of data
#define NULLABLE /// pointer can be null

/// macros for headers
#ifndef _POSIX_C_SOURCE
#   define _POSIX_C_SOURCE 200112L
#endif
