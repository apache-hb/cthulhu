#pragma once

#include <stdbool.h>
#include <stddef.h>

#include <gmp.h>

#include "ctu/util/util.h"

#include "ctu/ast/scanner.h"

typedef enum {
    /** 
     * # Builtin Integer types 
     * 
     * ## Range Integers
     * integers that are only required to be able to represent a range of values.
     * 
     * ### Signed
     *  - char. can represent at least the range [-128 to 127]
     *  - short. can represent at least the range [-32,768 to 32,767]
     *  - int. can represent at least the range [-2,147,483,648 to 2,147,483,647]
     *  - long. can represent at least the range [-9,223,372,036,854,775,807 to 9,223,372,036,854,775,807]
     * 
     * ### Unsigned
     *  - uchar. can represent at least the range [0 to 255]
     *  - ushort. can represent at least the range [0 to 65,535]
     *  - uint. can represent at least the range [0 to 4,294,967,295]
     *  - ulong. can represent at least the range [0 to 18,446,744,073,709,551,615]
     * 
     * ## Platform Integers
     * integers whos definition is platform dependant.
     * 
     * ### Signed
     *  - isize. a signed integer of equal size to `usize`
     *  - intptr. a signed integer of equal size to `uintptr`
     *  - imax. the largest available signed integer for the current platform.
     * 
     * ### Unsigned
     *  - usize. an unsigned integer that can represent the size of any type
     *  - uintptr. an unsigned integer that can represent the address of any pointer
     *  - umax. the largest available unsigned integer for the current platform.
     * 
     * ## Width Integers
     * integers that are required to be a specific size or in a range of sizes
     * 
     * ### Signed
     *  - intN. a signed integer that is exactly N bits wide
     *  - intNleast. a signed integer that is at least N bits wide
     *  - intNfast. a signed integer that is at least N bits wide
     * 
     * ### Unsigned
     *  - uintN. an unsigned integer that is exactly N bits wide
     *  - uintNleast. an unsigned integer that is at least N bits wide
     *  - uintNfast. an unsigned integer that is at least N bits wide
     * 
     * ## Behaviour
     *  - integer overflow and overflow are implementation defined.
     *  - narrowing conversions where the value previously held
     *    cannot be represented by the new type is undefined behaviour.
     *  - widening conversions of signed types perform sign extension 
     *    and widening conversions of unsigned types perform zero extension.
     *  - casting a signed type containing a negative number 
     *    to an unsgined type is undefined behaviour.
     */
    TYPE_INTEGER,

    /**
     * the result of a comparison
     */
    TYPE_BOOLEAN,

    /**
     * void type
     */
    TYPE_VOID,

    /**
     * the type of a function
     */
    TYPE_CALLABLE,

    /**
     * a pointer type
     */
    TYPE_POINTER,

    /* an array of a type, possibly with size */
    TYPE_ARRAY,

    /**
     * a struct
     */
    TYPE_STRUCT,

    /* a union */
    TYPE_UNION,

    /**
     * an enumeration
     */
    TYPE_ENUM,

    /**
     * a single field in a struct
     */
    TYPE_FIELD,

    /**
     * a string
     */
    TYPE_STRING,

    /* builtins */
    TYPE_SIZEOF,

    /**
     * error handling types
     */
    TYPE_POISON, /* typecheck error */
    TYPE_UNRESOLVED /* symbol lookup failed */
} typeof_t;

typedef enum {
    INTEGER_CHAR,
    INTEGER_SHORT,
    INTEGER_INT,
    INTEGER_LONG,
    INTEGER_SIZE,
    INTEGER_INTPTR,
    INTEGER_INTMAX,

    /* unused */
    INTEGER_END
} integer_t;

typedef struct {
    const char *name;
    struct type_t *type;
    struct node_t *init;
} field_t;

field_t new_type_field(const char *name, struct type_t *type);

typedef struct {
    size_t size;
    field_t *fields;
} fields_t;

typedef struct type_t {
    /** 
     * the type of this type 
     */
    typeof_t kind;
    
    /** 
     * is this type mutable 
     */
    bool mut:1;

    /** 
     * is this type signed or unsigned 
     * only used with TYPE_INTEGER
     */
    bool sign:1;

    /**
     * can we take a reference to this or assign to it
     */
    bool lvalue:1;

    /**
     * is this a type defined externally
     */
    bool interop:1;

    /* is this struct valid */
    bool valid:1;

    /* is this array unbounded */
    bool unbounded:1;

    /** 
     * the node that generated this type 
     */
    struct node_t *node;

    /**
     * index of this type in the type table
     */
    size_t index;

    /* data about this type */
    union {
        /**
         * poison error message
         */
        const char *text;
        
        /** 
         * builtin integer data 
         * TYPE_INTEGER
         */
        integer_t integer;

        /**
         * the type this type points to
         * TYPE_POINTER
         */
        struct type_t *ptr;

        /**
         * struct data
         */
        struct {
            const char *name;

            fields_t fields;
        };

        /* array data */
        struct {
            struct type_t *of;

            size_t size;
        };

        /**
         * callable data
         */
        struct {
            /* the original function */
            struct node_t *function;
            /* the return type */
            struct type_t *result;
            /* the argument types */
            list_t *args;
        };
    };
} type_t;

extern type_t *STRING_TYPE;
extern type_t *BOOL_TYPE;
extern type_t *VOID_TYPE;

type_t *get_int_type(bool sign, integer_t kind);

void types_init(void);

type_t *new_integer(integer_t kind, bool sign, const char *name);
type_t *new_builtin(typeof_t kind, const char *name);
type_t *new_unresolved(struct node_t *symbol);
type_t *new_poison(struct node_t *parent, const char *err);
type_t *new_callable(struct node_t *func, list_t *args, type_t *result);
type_t *new_pointer(struct node_t *node, type_t *to);
type_t *new_array(struct node_t *node, type_t *of, size_t size, bool unbounded);

type_t *new_struct(struct node_t *decl, const char *name);
type_t *new_union(struct node_t *decl, const char *name);
type_t *new_enum(struct node_t *decl, const char *name);

void resize_type(type_t *type, size_t size);

bool is_integer(type_t *type);
bool is_boolean(type_t *type);
bool is_callable(type_t *type);
bool is_void(type_t *type);
bool is_signed(type_t *type);
bool is_pointer(type_t *type);
bool is_const(type_t *type);
bool is_struct(type_t *type);
bool is_union(type_t *type);
bool is_array(type_t *type);
bool is_record(type_t *type);
bool can_index(type_t *type);
bool is_enum(type_t *type);
type_t *index_type(type_t *type);
type_t *array_decay(type_t *type);

type_t *get_result(type_t *func);

bool types_equal(type_t *type, type_t *other);

integer_t get_integer_kind(type_t *type);

type_t *set_mut(type_t *type, bool mut);
type_t *set_lvalue(type_t *type, bool lvalue);

void connect_type(struct node_t *node, type_t *type);

void sanitize_range(type_t *type, mpz_t it, scanner_t *scanner, where_t where);

bool type_can_become_implicit(struct node_t **node, type_t *dst, type_t *src);
bool type_can_become_explicit(struct node_t **node, type_t *dst, type_t *src);

char *typefmt(type_t *type);
struct node_t *nodeof(type_t *type);

type_t *builtin_sizeof(struct node_t *node, type_t *it);
type_t *size_int(void);
