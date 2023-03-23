%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cmd}

%code top {
    #include "interop/flex.h"
    #include <string.h>
}

%code requires {
    #include "argparse/common.h"
    #define YYSTYPE CMDSTYPE
    #define YYLTYPE CMDLTYPE
}

%{
int cmdlex(void *lval, void *loc, scan_t *scan);
void cmderror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    param_t *option;
    char *ident;
    mpz_t number;
}

%token<option>
    STRING_OPT "string option"
    INT_OPT "integer option"
    FLAG_OPT "flag option"

%token
    ERROR "unknown flag"

%token<ident>
    IDENT "identifier"

%token<number>
    NUMBER "number"

%token
    ASSIGN "="

%type<ident> ident
%type<number> number

%start entry

%%

entry: %empty | arguments ;
arguments: argument | arguments argument ;

argument: STRING_OPT ident { argparse_string_opt(scan_get(x), $1, $2); }
    | INT_OPT number { argparse_int_opt(scan_get(x), $1, $2); }
    | FLAG_OPT { argparse_flag_opt(scan_get(x), $1); }
    | ident { argparse_add_file(scan_get(x), $1); }
    | ERROR
    ;

ident: IDENT { $$ = $1; }
    | ASSIGN IDENT { $$ = $2; }
    ;

number: NUMBER { memcpy($$, $1, sizeof(mpz_t)); }
    | ASSIGN NUMBER { memcpy($$, $2, sizeof(mpz_t)); }
    ;

%%
