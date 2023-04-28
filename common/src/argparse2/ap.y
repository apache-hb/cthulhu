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
    ap_param_t *option;
    char *ident;
    char *error;
    mpz_t number;
}

%token<option>
    AP_STRING "string option"
    AP_INT "integer option"
    AP_BOOL "flag option"

%token<error>
    AP_ERROR "unknown flag"

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

argument: AP_STRING ident { ap_on_string(x, @$, $1, $2); }
    | AP_INT number { ap_on_int(x, @$, $1, $2); }
    | AP_BOOL { ap_on_bool(x, @$, $1, true); }
    | ident { ap_on_posarg(x, @$, $1); }
    | AP_ERROR { ap_on_error(x, @$, $1); }
    ;

ident: IDENT { $$ = $1; }
    | ASSIGN IDENT { $$ = $2; }
    ;

number: NUMBER { memcpy($$, $1, sizeof(mpz_t)); }
    | ASSIGN NUMBER { memcpy($$, $2, sizeof(mpz_t)); }
    ;

%%
