%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t x }
%locations
%expect 0
%define api.prefix {cmd}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "argparse/common.h"
    #define YYSTYPE CMDSTYPE
    #define YYLTYPE CMDLTYPE
}

%{
int cmdlex();
void cmderror(where_t *where, void *state, scan_t scan, const char *msg);
%}

%union {
    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"
    OPT "-"

%token<number>
    NUMBER "number"

%token
    ASSIGN "="

%start entry

%%

entry: %empty | arguments ;
arguments: argument | arguments argument ;

argument: flag
    | option
    | flag ASSIGN option
    ;

flag: OPT { argparse_begin_flag(scan_get(x), $1); }
    ;

option: IDENT { argparse_push_string(scan_get(x), $1); }
    | NUMBER { argparse_push_digit(scan_get(x), $1); }
    ;

%%
