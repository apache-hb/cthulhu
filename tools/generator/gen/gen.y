%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {gen}

%code top {
    #include "cthulhu/ast/interop.h"
}

%code requires {
    #include "gen.h"
    
    #define YYSTYPE GENSTYPE
    #define YYLTYPE GENLTYPE
}

%{
int genlex();
void generror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *text;
    vector_t *vector;

    ast_t *ast;
}

%type<vector>
    optComments comments

%type<ast>
    comment

%token<text>
    STRING "string literal"

%token
    LINE_COMMENT "line-comment"
    BLOCK_COMMENT "block-comment"
    KEYWORDS "keywords"

%start grammar

%%

grammar: optComments { scan_set(x, gen_grammar(x, @$, $1)); }
    ;

optComments: %empty { $$ = vector_new(0); }
    | comments { $$ = $1; }
    ;

comments: comment { $$ = vector_init($1); }
    | comment comments { vector_push(&$1, $2); $$ = $1; }
    ;

comment: LINE_COMMENT STRING { $$ = gen_line_comment(x, @$, $1); }
    | BLOCK_COMMENT STRING STRING { $$ = gen_line_comment(x, @$, $2, $3); }
    ;

%%
