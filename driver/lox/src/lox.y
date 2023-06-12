%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {lox}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "scan.h"
    #include "ast.h"
    
    #define YYSTYPE LOXSTYPE
    #define YYLTYPE LOXLTYPE
}

%{
int loxlex(void *lval, void *loc, scan_t *scan);
void loxerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;
    mpz_t number;
}

%start program

%%

%%
