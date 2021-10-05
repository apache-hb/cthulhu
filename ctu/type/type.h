#pragma once

#include <stdbool.h>

#include "ctu/util/util.h"

typedef enum {
    TY_LITERAL_INTEGER, /// integer literal type that can cast to any integer type

    TY_ANY, /// the any type, common with all other types
    TY_VOID, /// the unit type
    TY_BOOL, /// the boolean type
    TY_INTEGER, /// any integer type
    TY_PTR, /// a pointer to another type
    TY_CLOSURE, /// a function signature
    TY_STRING, /// a string type, convertible to const char*
    TY_VARARGS, /// a variadic function signature
    TY_POISON /// a compiler error
} metatype_t;

typedef enum {
    TY_CHAR, /// at least 8 bits wide
    TY_SHORT, /// at least 16 bits wide
    TY_INT, /// at least 32 bits wide
    TY_LONG, /// at least 64 bits wide
    TY_SIZE, /// used for indexing
    TY_INTPTR, /// the same width as a pointer type
    TY_INTMAX /// the largest available integer type
} int_t;

typedef enum {
    SIGNED, /// signed integer
    UNSIGNED /// unsigned integer
} sign_t;

typedef struct {
    sign_t sign;
    int_t kind;
} digit_t;

typedef struct {
    size_t align;
} extra_t;

typedef struct type_t {
    metatype_t type;

    /* is this type mutable */
    bool mut;

    union {
        digit_t digit;

        struct {
            vector_t *args;
            struct type_t *result;
        };

        const char *msg;

        const struct type_t *ptr;
    };
} type_t;


/**
 * format a type into a string
 * 
 * @param type the type to format
 * @return the formatted string
 */
char *type_format(const type_t *type);


/**
 * create a integer literal type that is common to any integer
 * 
 * @return the integer literal type
 */
type_t *type_literal_integer(void);

/**
 * create the any type
 *
 * @return the any type
 */
const type_t *type_any(void);

/**
 * create a digit type
 * 
 * @param sign the sign of the integer @see sign_t
 * @param kind the width of the integer @see int_t
 * @return the digit type
 */
type_t *type_digit(sign_t sign, int_t kind);

/**
 * create a void type
 * 
 * @return the void type
 */
type_t *type_void(void);

/**
 * create a callable type
 * 
 * @param args a vector_t<type_t*> of the arguments that this function takes. @see vector_t
 * @param result the return type of this function
 * @return a callable type
 */
type_t *type_closure(vector_t *args, type_t *result);

/**
 * create a pointer type
 * 
 * @param to the type this pointer points to
 * @return a pointer type
 */
type_t *type_ptr(const type_t *to);

/**
 * create a string type
 * 
 * @return the string type
 */
type_t *type_string(void);

/**
 * create the boolean type
 * 
 * @return the boolean type
 */
type_t *type_bool(void);

/**
 * create an untyped varargs type
 * 
 * @return the varargs type
 */
type_t *type_varargs(void);

/**
 * create a poison type
 * 
 * @param msg the error message that casued this type
 * @return the poison type with message
 */
type_t *type_poison(const char *msg);

void type_mut(type_t *type, bool mut);

bool is_const(const type_t *type);
bool is_any(const type_t *type);
bool is_literal(const type_t *type);
bool is_integer(const type_t *type);
bool is_digit(const type_t *type);
bool is_bool(const type_t *type);
bool is_signed(const type_t *type);
bool is_unsigned(const type_t *type);
bool is_void(const type_t *type);
bool is_poison(const type_t *type);
bool is_pointer(const type_t *type);
bool is_closure(const type_t *type);
bool is_varargs(const type_t *type);
bool is_variadic(const type_t *type);
size_t minimum_params(const type_t *type);
const type_t *closure_result(const type_t *type);
bool is_string(const type_t *type);

/**
 * return a common type of lhs and rhs if possible.
 * e.g. (int, char) -> int
 *      (bool, int) -> poison
 *      (uint, int) -> int
 * 
 * @param lhs the left hand side type
 * @param rhs the right hand side type
 * @return the common type of lhs and rhs
 */
const type_t *types_common(const type_t *lhs, const type_t *rhs);
