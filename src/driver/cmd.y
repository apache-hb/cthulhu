%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cmd}

%code top {
    #include "cthulhu/ast/interop.h"
}

%code requires {
    #include "src/driver/cmd.h"
    #define YYSTYPE CMDSTYPE
    #define YYLTYPE CMDLTYPE
}

%{
#include "src/driver/cmd.h"
int cmdlex();
void cmderror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"
    PATH "path"
    LONG_OPT "--"
    SHORT_OPT "-"

%token<number>
    NUMBER "number"

%token
    ASSIGN "="

%type<ident>
    file

%start entry

%%

entry: %empty | arguments ;
arguments: argument | arguments argument ;

argument: file { cmd_add_file(scan_get(x), $1); }
    | flag { }
    | flag option { }
    | flag ASSIGN option { }
    ;

file: PATH ;

flag: LONG_OPT { }
    | SHORT_OPT { }
    ;

option: IDENT { }
    | NUMBER { }
    ;

%%
