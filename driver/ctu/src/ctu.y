%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ctu}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "ctu/ast.h"
    #include "ctu/scan.h"
    #define YYSTYPE CTUSTYPE
    #define YYLTYPE CTULTYPE
}

%{
int ctulex(void *lval, void *loc, scan_t *scan);
void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;

    vector_t *vector;

    ast_t *ast;
}

%token<ident>
    IDENT "identifier"

%token
    MODULE "module"
    IMPORT "import"
    EXPORT "export"

    SEMI ";"
    COLON2 "::"

%type<vector>
    path modspec

%%

program: modspec { scan_set(x, ast_module(x, @$, $1)); }
    ;

modspec: %empty { $$ = vector_of(0); }
    | MODULE path SEMI { $$ = $2; }
    ;

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

%%