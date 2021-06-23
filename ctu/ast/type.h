#pragma once

#include <stdbool.h>

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
     *  - long. can represent at least the range [-9,223,372,036,854,775,808 to 9,223,372,036,854,775,807]
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
     * the type of a function
     */
    TYPE_CALLABLE,

    /**
     * error handling type
     */
    TYPE_POISON
} typeof_t;

typedef enum {
    INTEGER_CHAR,
    INTEGER_SHORT,
    INTEGER_INT,
    INTEGER_LONG,
    INTEGER_SIZE,
    INTEGER_INTPTR,
    INTEGER_INTMAX
} integer_t;

typedef struct {
    struct type_t *data;
    size_t size;
} types_t;

typedef struct type_t {
    /** 
     * the type of this type 
     */
    typeof_t kind;
    
    /** 
     * is this type mutable 
     * unused with TYPE_VOID
     */
    bool mut:1;

    /** 
     * is this type signed or unsigned 
     * only used with TYPE_INTEGER
     */
    bool sign:1;

    /** 
     * the node that generated this type 
     */
    struct node_t *node;

    /* data about this type */
    union {
        /**
         * poison error message
         */
        char *text;
        
        /** 
         * builtin integer data 
         * TYPE_INTEGER
         */
        integer_t integer;

        /**
         * the original function if this is a callable
         */
        struct {
            struct node_t *function;
            struct type_t *result;
            types_t args;
        };
    };
} type_t;
