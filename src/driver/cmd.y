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
    #define CMDLTYPE_IS_DECLARED
}

%{
#include "src/driver/cmd.h"
int cmdlex();
void cmderror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    option_t *option;
    cmd_flag_t flag;

    const char *ident;
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

    ENABLE "enable"
    DISABLE "disable"

%type<ident>
    file

%type<flag>
    flag

%type<option>
    option

%start entry

%%

entry: %empty | arguments ;
arguments: argument | arguments argument ;

argument: file { add_file($1); }
    | flag { add_option($1, NULL); }
    | flag option { add_option($1, $2); }
    | flag ASSIGN option { add_option($1, $3); }
    ;

file: PATH ;

flag: LONG_OPT { $$ = get_flag($1); }
    | SHORT_OPT { $$ = get_flag($1); }
    ;

option: IDENT { $$ = string_option($1); }
    | NUMBER { $$ = number_option($1); }
    | ENABLE { $$ = bool_option(true); }
    | DISABLE { $$ = bool_option(false); }
    ;

%%
