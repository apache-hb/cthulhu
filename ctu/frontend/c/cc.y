%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cc}

%code requires {
    #define YYSTYPE CCSTYPE
    #define YYLTYPE CCLTYPE

    #include "scan.h"
    #include "ast.h"
}

%{
#include "scan.h"
int cclex();
void ccerror(where_t *where, void *state, scan_t *scan, const char *msg);    
%}

%union {
    c_t *c;

    char *ident;
    mpz_t integer;
}

%token<ident>
    IDENT "identifier"

%token<integer>
    INTEGER "integer literal"

%token
    CONST "const"

%start program

%%

program: %empty ;

%%
