#ifndef CTU_H
#define CTU_H

#include "thirdparty/vec/vec.h"
#include "thirdparty/map/map.h"

#include <stdlib.h>
#include <stdint.h>

typedef void* ctu_file;

typedef int(*ctu_next)(ctu_file);
typedef int(*ctu_peek)(ctu_file);
typedef void(*ctu_seek)(ctu_file, size_t);

typedef struct {
    ctu_next next;
    ctu_peek peek;
    ctu_seek seek;

    ctu_file handle;
} ctu_input;

typedef enum {
    ctu_tt_str,
    ctu_tt_ident,
    ctu_tt_char,
    ctu_tt_int,
    ctu_tt_num,
    ctu_tt_key,
    ctu_tt_eof
} ctu_token_type;

typedef enum {

#define KEYWORD(id, str) id,
#define OPERATOR(id, str) id,
#define ASM_KEYWORD(id, str) id,

#include "keywords.inc"

} ctu_keyword;

typedef struct {
    ctu_token_type type;

    union {
        char* str;
        char* ident;
        char ch;
        int64_t integer;
        double number;
        ctu_keyword key;
    };
} ctu_token;

void ctu_free_token(ctu_token tok);

typedef struct {
    ctu_input input;

    int64_t line;
    int64_t col;
    int64_t pos;

    ctu_token tok;
} ctu_lexer;

ctu_lexer ctu_new_lexer(ctu_input input);
void ctu_free_lexer(ctu_lexer* lexer);

ctu_token ctu_lexer_next(ctu_lexer* lexer);
ctu_token ctu_lexer_peek(ctu_lexer* lexer);

void ctu_print_token(ctu_token tok);

typedef struct {
    ctu_lexer lex;
} ctu_parser;

typedef vec_str_t ctu_path;

typedef enum {
    ctu_et_binary,
    ctu_et_unary,
    ctu_et_call,
} ctu_expr_type;

typedef struct {
    ctu_expr_type type;
} ctu_expr;

typedef map_t(ctu_expr) ctu_expr_map;

typedef enum {
    ctu_at_asm,
    ctu_at_bitfield,
} ctu_attribute_type;

typedef struct {
    ctu_attribute_type type;

    union {
        struct {
            /* the starting bit of the bitfield 
             if -1 then the bf starts at the end of the last variable */
            int_fast16_t bf_start;

            /* the number of bits in the bitfield */
            int_fast16_t bf_length;
        };
    };
} ctu_attribute;

typedef vec_t(ctu_attribute) ctu_attrib_vec;

typedef enum {
    ctu_struct,
    ctu_enum,
    ctu_variant,
    ctu_alias,
    ctu_array,
    ctu_ptr,
    ctu_builtin,
} ctu_type_type;

typedef enum {
    ctu_void,
    ctu_u8,
    ctu_u16,
    ctu_u32,
    ctu_u64,
    ctu_u128,
    ctu_i8,
    ctu_i16,
    ctu_i32,
    ctu_i64,
    ctu_i128,
    ctu_b8,
    ctu_b16,
    ctu_b32,
    ctu_b64,
    ctu_f32,
    ctu_f64
} ctu_builtin_type;

typedef struct ctu_type_tag {
    ctu_type_type type;

    ctu_attrib_vec attribs;

    union {
        /* builtin */
        ctu_builtin_type b_type;

        /* pointer */
        struct ctu_type_tag* p_type;

        /* struct */
        struct ctu_type_map* s_fields;

        /* fixed size array */
        struct {
            struct ctu_type_tag* ar_type;
            ctu_expr ar_size;
        };

        /* alias */
        struct ctu_type_tag* al_type;

        /* variant */
        struct ctu_type_map* v_fields;

        /* enum */
        ctu_expr_map e_fields;
    };
} ctu_type;

typedef struct {
    char* name;
    ctu_type type;
} ctu_using;

typedef map_t(ctu_type) ctu_type_map;

#endif /* CTU_H */