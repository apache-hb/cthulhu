#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>

typedef struct {
    // data pointer
    void* data;

    // get the next charater
    int(*next)(void*);

    // peek the next character
    int(*peek)(void*);

    // cleanup the data
    void(*close)(void*);
} file_t;

typedef enum {
    // top level keywords
    kw_using, // using
    kw_module, // module
    kw_scope, // scope
    kw_def, // def

    // decl keywords
    kw_return, // return
    kw_let, // let
    kw_var, // var
    kw_while, // while
    kw_for, // for

    // expr keywords
    kw_match, // match
    kw_if, // if
    kw_else, // else

    // type keywords
    kw_union, // union
    kw_enum, // enum

    // ops
    op_bitand, // &
    op_bitandeq, // &=

    op_bitor, // |
    op_bitoreq, // |=

    op_bitxor, // ^
    op_bitxoreq, // ^=

    op_bitnot, // ~

    op_shl, // <<
    op_shleq, // <<=

    op_shr, // >>
    op_shreq, // >>=

    op_and, // &&
    op_or, // ||
    op_eq, // ==
    op_neq,  // !=

    op_gt, // >
    op_gte, // >=

    op_lt, // <
    op_lte, // <=

    op_not, // !

    op_add, // +
    op_addeq, // +=

    op_sub, // -
    op_subeq, // -=

    op_mul, // *
    op_muleq, // *=

    op_div, // /
    op_diveq, // /=

    op_mod, // %
    op_modeq, // %=

    op_openarg, // (
    op_closearg, // )

    op_openscope, // {
    op_closescope, // }

    op_openarr, // [
    op_closearr, // ]

    op_assign, // =
    op_sep, // ::
    op_colon, // :
    op_arrow, // ->
    op_func, // &(
} keyword_e;

typedef enum {
    tt_str = (1 << 0),
    tt_ident = (1 << 1),
    tt_char = (1 << 2),
    tt_keyword = (1 << 3),
    tt_int = (1 << 4),
    tt_float = (1 << 5),
    tt_eof = (1 << 6),
} token_type_e;

typedef struct {
    token_type_e type;

    union {
        // string or ident
        char* str;

        // floating point type
        double flt;

        // number type
        int64_t num;

        // keyword
        keyword_e key;

        // letter
        char letter;
    };
} token_t;

void token_free(token_t* self);

typedef struct {
    file_t file;
    int top;
    char* buf;

    token_t tok;
} lexer_t;

lexer_t lexer_alloc(file_t self);
void lexer_free(lexer_t* self);

token_t lexer_next(lexer_t* self);
token_t lexer_peek(lexer_t* self);

#if 0

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
    kw_let = 52, // let
    kw_var = 53, // var

    op_func = 54, // &(
    kw_enum = 55, // enum
    kw_union = 56, // union

    op_question = 57, // ?

    kw_none = 0xFF
} keyword_t;

typedef enum {
    string = 0,
    letter = 1,
    integer = 2,
    floating = 3,
    keyword = 4,
    ident = 5,
    eof = 6,
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
    using_type_decl = 0,

    scope_decl = 1,

    dotted_name_decl = 2,

    body_decl = 3,

    type_name_decl = 4,

    type_decl = 5,

    field_decl = 6,

    named_type_list_decl = 7,

    type_list_decl = 8,

    func_sig_decl = 9,

    array_decl = 10,

    enum_decl,
    enum_field_decl,
    typed_enum_decl,

    union_decl,
    union_tuple_decl,

    expr_add, // op + op
    expr_addeq, // op += op
    expr_sub, // op - op
    expr_subeq, // op -= op

    expr_not, // !op
    expr_neg, // -op
    expr_pos, // +op

    expr_mul, // op * op
    expr_muleq, // op *= op
    expr_div, // op / op
    expr_diveq, // op /= op
    expr_mod, // op % op
    expr_modeq, // op %= op

    expr_bitand, // op & op
    expr_bitandeq, // op &= op

    expr_ref, // & op
    expr_deref, // * op

    expr_bitor, // op | op
    expr_bitoreq, // op |= op
    
    expr_bitxor, // op ^ op
    expr_bitxoreq, // op ^= op

    expr_bitnot, // ~op

} node_type_t;

typedef struct node_tag_t {
    node_type_t node_type;

    union {
        /////////////////////////////////////
        ///             types
        /////////////////////////////////////

        // using_type_decl
        struct {
            // name of the type
            char* name;

            // the type of the type
            struct node_tag_t* type;
        } using_type_decl;

        struct {
            // the underlying type
            struct node_tag_t* data;

            // is the type a pointer
            int pointer;
        } type_decl;

        struct {
            // number of fields
            int count;
            
            // array of name:type pairs
            struct node_tag_t* types;
        } named_type_list_decl;

        struct {
            // name of the field
            char* name;

            // type of the field
            struct node_tag_t* type;
        } field_decl;

        struct {
            // length of type list
            int count;

            // array of types
            struct node_tag_t* types;
        } type_list_decl;

        struct {
            // the arguments of the function
            struct node_tag_t* args;
            struct node_tag_t* ret_type;
        } func_sig_decl;

        struct {
            struct node_tag_t* type;

            // expression for array size, NULL if array is unsized
            struct node_tag_t* size;
        } array_decl;

        struct {
            // underlying type
            struct node_tag_t* backing;

            // number of name:type pairs
            int field_count;
            struct node_tag_t* fields;
        } typed_enum_decl;

        struct {
            // number of name:type pairs
            int field_count;
            struct node_tag_t* fields;
        } union_decl;

        struct {
            int count;
            char** names;
        } enum_decl;

        struct {
            // name:expr pairs
            int count;
            struct node_tag_t* fields;
        } valued_enum_decl;

        ////////////////////////////////////
        ///       expression stuff
        ////////////////////////////////////

        struct {
            struct node_tag_t* lhs;
            struct node_tag_t* rhs;
        } binop_expr;

        struct {
            struct node_tag_t* op;
        } unary_expr;

        ////////////////////////////////////
        ///         scoping stuff
        ////////////////////////////////////

        // scope_decl
        struct {
            // name of the scope
            struct node_tag_t* name;

            // decls inside the scope
            struct node_tag_t* decls;
        } scope_decl;

        // scope_name_decl
        struct {
            // number of parts in the name
            int count;

            // array of parts in the name
            char** parts;
        } dotted_name_decl;

        // body_decl
        struct {
            // number of decls in body
            int count;

            // array of count decls
            struct node_tag_t* decls;
        } body_decl;
    };
} node_t;

void node_free(node_t* self);

typedef struct {
    lexer_t* source;
} parser_t;

parser_t* parser_alloc(lexer_t* lex);
void parser_free(parser_t* self);

node_t* parser_generate_ast(parser_t* self);

typedef struct {
    int top;
    char** names;
} name_stack_t;

void name_stack_push(name_stack_t* self, char* name);
char* name_stack_pop(name_stack_t* self);

typedef struct {
    int top;
    node_t** nodes;
} node_stack_t;

void node_stack_push(node_stack_t* self, node_t* node);
node_t* node_stack_pop(node_stack_t* self);

#endif 

#endif // COMMON_H
