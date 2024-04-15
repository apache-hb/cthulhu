%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {meta}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"
}

%code requires {
    #include "meta/ast.h"
    #include "meta/scan.h"
    #define YYSTYPE STYPE
    #define YYLTYPE LTYPE
}

%{
int metalex(void *lval, void *loc, scan_t *scan);
void metaerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%start program

%%

program: %empty ;

%%
