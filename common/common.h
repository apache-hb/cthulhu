#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdint.h>

/*
quick grammar

functions are

def name(arg1: type1, arg2: type2) -> type3 {
    body
}

scopes are 

scope name {
    anything
}

variables are

let name: type
let name: type = expr
let name = expr

var name: type
var name: type = expr
var name = expr

*/

// all keywords
// if you update this make sure to update lexer.c to reflect new keywords
typedef enum {
    kw_for = 0, // for
    kw_if = 1, // if
    kw_else = 2, // else
    kw_def = 3, // def
    kw_while = 4, // while
    kw_switch = 5, // switch
    kw_case = 6, // case

    op_assign = 7, // =
    
    op_add = 8, // +
    op_addeq = 9, // +=
    
    op_sub = 10, // -
    op_subeq = 11, // -=

    op_div = 12, // /
    op_diveq = 13, // /=

    op_mul = 14, // *
    op_muleq = 15, // *=

    op_mod = 16, // %
    op_modeq = 17, // %=

    op_bitor = 18, // |
    op_bitoreq = 19, // |=

    op_bitand = 20, // &
    op_bitandeq = 21, // &=

    op_shl = 22, // <<
    op_shleq = 23, // <<=
    
    op_shr = 24, // >>
    op_shreq = 25, // >>=

    op_bitxor = 26, // ^
    op_bitxoreq = 27, // ^=

    op_bitnot = 28, // ~
    op_bitnoteq = 29, // ~=

    op_eq = 30, // ==
    op_neq = 31, // !=

    op_not = 32, // !
    op_and = 33, // &&
    op_or = 34, // ||
    
    op_sep = 35, // ::
    op_openarg = 36, // (
    op_closearg = 37, // )
    op_openscope = 38, // {
    op_closescope = 39, // }
    op_openarr = 40, // [
    op_closearr = 41, // ]
    op_comma = 42, // ,
    op_arrow = 43, // ->
    op_colon = 44, // :

    op_gt = 45, // >
    op_gte = 46, // >=
    op_lt = 47, // <
    op_lte = 48, // <=

    kw_scope = 49, // scope
    kw_return = 50, // return
    kw_using = 51, // using
} keyword_t;

typedef enum {
    string = 0,
    letter = 1,
    integer = 2,
    floating = 3,
    keyword = 4,
    ident = 5,
    eof = 6
} token_type_t;

typedef struct {
    token_type_t type;
    uint64_t row;
    uint64_t col;
    uint64_t cursor;

    union
    {
        keyword_t key;
        char* str;
    };
} token_t;

void token_free(token_t tok);

// type that defines a file, allows in memory files
typedef struct {
    // pointer to a block of data
    void* data;
    
    // function that gets the next character
    char(*next)(void*);

    // function that peeks the next character
    char(*peek)(void*);
    
    // function that closes the file/frees the data
    void(*close)(void*);

    // function to seek in the file
    void(*seek)(void*, uint64_t);

    // function to tell current offset
    uint64_t(*tell)(void*);
} file_t;

typedef struct {
    // pointer to code source
    file_t* file;

    // current row
    uint64_t row;
    // current column
    uint64_t col;

    // cursor
    uint64_t cursor;

    uint32_t buf_cursor;
    // internal buffer for lexer use
    char* buf;

    // last token parsed, used for peek
    token_t tok;
} lexer_t;

lexer_t* lexer_alloc(file_t* file);
void lexer_free(lexer_t* self);

token_t lexer_next(lexer_t* self);
token_t lexer_peek(lexer_t* self);

typedef enum {
    scope_decl = 0,
    func_decl = 1,
} expr_type_t;

typedef enum {
    // structure type
    structure = 0,
    // enumeration type
    enumeration = 1,
    // tuple type
    tuple = 2,
    // array type
    array = 3,
    // builtin type
    builtin = 4,
    // pointer to type
    pointer = 5,
} typeof_type_t;

typedef enum {
    
} builtin_t;

typedef struct {
    char* name;
    type_t* type;
    int constness;
} struct_field_t;

typedef struct {
    typeof_type_t type;

    union {

        // struct or tuple type
        struct {
            int field_count;
            union {
                type_t* tuple_fields;
                struct_field_t* struct_fields;
            };
        };

        // builtin type
        builtin_t* builtin_type;

        // array type
        struct {
            // length of the array
            int array_length;
            type_t* array_of;
        };

        // pointer type
        type_t* points_to;
    };
} type_t;

typedef struct {
    char* name;
    type_t* typeof;
    int constness;
} arg_pair_t;

typedef struct {
    int arg_count;
    char** arg_names;
} args_t;

typedef struct {
    expr_type_t node_type;

    union {

        // namespace/scope expression
        struct {
            char* scope_name;
            node_t* content;
        };

        // type decl
        struct {
            char* type_name;
            type_t type;
        };
    };
} node_t;

typedef struct {
    lexer_t* source;
} parser_t;

parser_t* parser_alloc(lexer_t* lex);
void parser_free(parser_t* self);

node_t* parser_generate_ast(parser_t* self);

#endif // COMMON_H
