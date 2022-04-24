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
    flag_t *flag;
    option_t option;

    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"
    OPT "-"
    SEP

%token<number>
    NUMBER "number"

%token
    ASSIGN "="

%type<flag>
    flag

%type<option>
    option

%start entry

%%

entry: %empty | arguments ;
arguments: argument | arguments SEP argument ;

argument: IDENT { cmd_add_file(scan_get(x), $1); }
    | flag { cmd_set_flag(scan_get(x), $1); }
    | flag option { cmd_set_option(scan_get(x), $1, $2); }
    | flag ASSIGN option { cmd_set_option(scan_get(x), $1, $3); }
    ;

flag: OPT { $$ = cmd_get_flag(scan_get(x), $1); }
    ;

option: IDENT { $$ = option_ident($1); }
    | NUMBER { $$ = option_number($1); }
    ;

%%
