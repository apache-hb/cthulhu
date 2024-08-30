%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {sql}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"
}

%code requires {
    #include "sql/ast.h"
    #include "sql/scan.h"
    #define YYSTYPE STYPE
    #define YYLTYPE LTYPE
}

%{
int sqllex(void *lval, void *loc, scan_t *scan);
void sqlerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%start program

%%

program: %empty ;

%%
