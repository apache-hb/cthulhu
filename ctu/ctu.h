#ifndef CTU
#define CTU

#include <stdint.h>

#include "keywords.h"

#include "utils/vec/vec.h"
#include "utils/map/map.h"

typedef struct {
    void* handle;
    int(*next)(void*);
} ctu_file;

#define TOK_EOF 0
#define TOK_IDENT 1
#define TOK_STRING 2
#define TOK_KEY 3
#define TOK_INT 4
#define TOK_FLOAT 5
#define TOK_CHAR 6

typedef int ctu_tok_type;

typedef struct {
    vec_char_t* file;

    /* the real distance to the line the token is on */
    size_t distance;
    
    /* the column of the first token character */
    size_t col;

    /* the line the token is on */
    size_t line;
} ctu_position;

typedef struct {
    ctu_tok_type type;

    ctu_position pos;

    /* length of the token */
    size_t length;

    union {
        /* TOK_IDENT || TOK_STRING */
        char* string;

        /* TOK_KEY */
        ctu_keyword keyword;

        /* TOK_INT */
        int64_t integer;

        /* TOK_FLOAT */
        double number;

        /* TOK_CHAR */
        int ch;
    };
} ctu_token;

void ctu_token_delete(ctu_token);

typedef struct {
    ctu_file file;
    vec_char_t buffer;
    ctu_token tok;
    int ch;

    size_t line;
    size_t col;
    size_t distance;
} ctu_lexer;

ctu_lexer ctu_lexer_new(ctu_file);
void ctu_lexer_delete(ctu_lexer*);

ctu_token ctu_lexer_next(ctu_lexer*);
ctu_token ctu_lexer_peek(ctu_lexer*);

typedef struct {
    ctu_lexer* lex;
    int preamble;
} ctu_parser;

ctu_parser ctu_parser_new(ctu_lexer*);

typedef enum {
    nt_dotted,
    nt_struct,
    nt_tuple,
    nt_union,
    nt_variant,
    nt_enum,
    nt_array,
    nt_ptr,
    nt_funcsig,
    nt_builtin,
    nt_typename,
    nt_typedef,
    nt_funcdef,
    nt_import,
    nt_scopedef
} ctu_node_type;

typedef enum {
    u8,
    u16,
    u32,
    u64,
    i8,
    i16,
    i32,
    i64,
    f32,
    f64,
    c8,
    _bool,
    _void,

} ctu_builtin;

typedef vec_t(struct ctu_node) ctu_node_vec;

typedef struct ctu_node_tag {
    ctu_node_type type;

    union {
        /* NT_IMPORT */
        vec_str_t i_path;

        /* NT_DOTTED */
        vec_str_t d_name;

        /* NT_STRUCT */
        struct ctu_node_map* s_fields;

        /* NT_TUPLE */
        ctu_node_vec t_fields;

        /* NT_UNION */
        struct ctu_node_map* u_fields;

        /* NT_VARIANT */
        struct ctu_node_map* v_fields;

        /* NT_ENUM */
        struct ctu_node_map* e_fields;

        /* NT_PTR */
        struct ctu_node_tag* p_type;

        /* NT_FUNCSIG */
        struct {
            /* argument types */
            ctu_node_vec f_args;

            /* return type */
            struct ctu_node_tag* f_return;
        };

        /* NT_BUILTIN */
        ctu_builtin b_type;

        /* NT_TYPENAME */
        vec_str_t t_name;

        /* NT_TYPEDEF */
        struct {
            /* type name */
            char* td_name;

            /* type */
            struct ctu_node_tag* td_type;
        };
    };
} ctu_node;

void ctu_node_free(ctu_node node);

ctu_node ctu_parser_next(ctu_parser* parse);

typedef map_t(ctu_node) ctu_node_map;

#endif /* CTU */
