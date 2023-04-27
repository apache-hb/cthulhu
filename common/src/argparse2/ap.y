%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ap}

%code top {
    #include "interop/flex.h"
    #include <string.h>
}

%code requires {
    #include "argparse2/common.h"
    #define YYSTYPE APSTYPE
    #define YYLTYPE APLTYPE
}

%{
int aplex(void *lval, void *loc, scan_t *scan);
void aperror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    ap2_param_t *option;
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

argument: STRING_OPT ident { ap2_on_string(scan_get(x), $1, $2); }
    | INT_OPT number { ap2_on_int(scan_get(x), $1, $2); }
    | FLAG_OPT { ap2_on_bool(scan_get(x), $1, true); }
    | ident { ap2_on_posarg(scan_get(x), $1); }
    | ERROR
    ;

ident: IDENT { $$ = $1; }
    | ASSIGN IDENT { $$ = $2; }
    ;

number: NUMBER { memcpy($$, $1, sizeof(mpz_t)); }
    | ASSIGN NUMBER { memcpy($$, $2, sizeof(mpz_t)); }
    ;

%%
