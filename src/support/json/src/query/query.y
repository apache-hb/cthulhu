%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {query}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"

    #include "std/typed/vector.h"
    #include "std/vector.h"
}

%code requires {
    #include "query_ast.h"
    #include "query_scan.h"
    #define YYSTYPE QUERYSTYPE
    #define YYLTYPE QUERYLTYPE
}

%{
int querylex(void *lval, void *loc, scan_t *scan);
void queryerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    mpz_t integer;
    text_t string;

    query_ast_t *ast;
}

%token
    LBRACKET "["
    RBRACKET "]"
    DOT "."

%token<string>
    STRING

%token<integer>
    INTEGER

%token<string>
    IDENT

%type<ast>
    root expr

%start root

%%

root: expr { scan_set(x, $1); }
    ;

expr: IDENT { $$ = query_ast_object(x, $1); }
    | expr DOT IDENT { $$ = query_ast_field(x, $1, $3); }
    | expr LBRACKET INTEGER RBRACKET { $$ = query_ast_index(x, $1, $3); }
    | expr LBRACKET STRING RBRACKET { $$ = query_ast_map(x, $1, $3); }
    ;

%%
