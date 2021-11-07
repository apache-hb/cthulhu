#pragma once

#include <stdbool.h>

#include "ctu/ast/ast.h"
#include "ctu/util/util.h"

typedef enum {
    /* untyped types */
    TY_LITERAL_INTEGER, /// integer literal type that can cast to any integer type
    TY_ANY, /// the any type, common with all other types
    
    /* builtin types */
    TY_VOID, /// the unit type
    TY_BOOL, /// the boolean type
    TY_INTEGER, /// any integer type
    TY_PTR, /// a pointer to another type
    TY_ARRAY, /// an array of a type
    TY_CLOSURE, /// a function signature
    TY_STRING, /// a string type, convertible to const char*
    TY_VARARGS, /// a variadic function signature

    /* user defined types */
    TY_STRUCT,
    TY_UNION,

    /* error handling types */
    TY_FORWARD, /// a forward declaration
    TY_POISON /// a compiler error
} metatype_t;

typedef enum {
    TY_CHAR, /// at least 8 bits wide
    TY_SHORT, /// at least 16 bits wide
    TY_INT, /// at least 32 bits wide
    TY_LONG, /// at least 64 bits wide
    TY_SIZE, /// used for indexing
    TY_INTPTR, /// the same width as a pointer type
    TY_INTMAX, /// the largest available integer type

    TY_INT_TOTAL /// tail
} int_t;

typedef enum {
    SIGNED, /// signed integer
    UNSIGNED, /// unsigned integer

    SIGN_TOTAL /// tail
} sign_t;

typedef struct {
    sign_t sign;
    int_t kind;
} digit_t;

typedef struct {
    const char *name;
    struct type_t *type;
} aggregate_field_t;

typedef struct type_t {
    metatype_t type;

    /* is this type mutable */
    bool mut:1;

    const char *name; /// the name of this type
    const node_t *node; /// the source location of this type

    union {
        digit_t digit;

        struct {
            vector_t *args;
            struct type_t *result;
        };

        struct {
            const struct type_t *ptr;
            bool index;
        };

        struct {
            const struct type_t *elements;
            size_t len;
        };

        vector_t *fields;

        void *data;
    };
} type_t;

aggregate_field_t *new_aggregate_field(const char *name, type_t *type);

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
type_t *type_any(void);

/**
 * create a digit type
 * 
 * @param sign the sign of the integer @see sign_t
 * @param kind the width of the integer @see int_t
 * @return the digit type
 */
type_t *type_digit(sign_t sign, int_t kind);
type_t *type_digit_with_name(const char *name, sign_t sign, int_t kind);
type_t *type_usize(void);

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
type_t *type_ptr_with_index(const type_t *ptr, bool index);

type_t *type_array(const type_t *element, size_t len);

/**
 * create a string type
 * 
 * @return the string type
 */
type_t *type_string(void);
type_t *type_string_with_name(const char *name);

/**
 * create the boolean type
 * 
 * @return the boolean type
 */
type_t *type_bool(void);
type_t *type_bool_with_name(const char *name);

/**
 * create an untyped varargs type
 * 
 * @return the varargs type
 */
type_t *type_varargs(void);

type_t *type_forward(const char *name, const node_t *node, void *data);

type_t *type_struct(const char *name, const node_t *node, vector_t *fields);

type_t *type_union(const char *name, const node_t *node, vector_t *fields);

/**
 * create a poison type
 * 
 * @param msg the error message that casued this type
 * @return the poison type with message
 */
type_t *type_poison(const char *msg);
type_t *type_poison_with_node(const char *msg, const node_t *node);

type_t *type_mut(const type_t *type, bool mut);

bool is_const(const type_t *type);
bool is_any(const type_t *type);
bool is_literal(const type_t *type);
bool is_integer(const type_t *type);
bool is_digit(const type_t *type);
bool is_bool(const type_t *type);
bool is_array(const type_t *type);
bool is_signed(const type_t *type);
bool is_unsigned(const type_t *type);
bool is_void(const type_t *type);
bool is_voidptr(const type_t *type);
bool is_poison(const type_t *type);
bool is_pointer(const type_t *type);
bool is_closure(const type_t *type);
bool is_varargs(const type_t *type);
bool is_variadic(const type_t *type);
size_t maximum_params(const type_t *type);
size_t minimum_params(const type_t *type);
size_t num_params(const type_t *type);
const type_t *param_at(const type_t *type, size_t idx);
const type_t *closure_result(const type_t *type);
vector_t *closure_params(const type_t *type);
bool is_string(const type_t *type);

bool is_aggregate(const type_t *type);
bool is_struct(const type_t *type);
bool is_union(const type_t *type);
size_t field_offset(const type_t *type, const char *name);
const type_t *get_field(const type_t *type, size_t idx);

const char *get_poison_type_message(const type_t *type);
const type_t *index_type(const type_t *type);

bool type_can_index(const type_t *type);
bool type_is_indirect(const type_t *type);

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

bool types_exact_equal(const type_t *lhs, const type_t *rhs);
