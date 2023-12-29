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
    #include "scan/node.h"
    #include "common.h"
    #define YYSTYPE APSTYPE
    #define YYLTYPE APLTYPE
}

%{
int aplex(void *lval, void *loc, scan_t *scan);
void aperror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    ap_field_t field;
    char *ident;
    char *error;
    mpz_t number;
}

%token<field>
    AP_STRING_OPTION "string option"
    AP_INT_OPTION "integer option"
    AP_BOOL_OPTION "boolean option"

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

argument: AP_STRING_OPTION ident { ap_on_string(x, $1.field, $2); }
    | AP_INT_OPTION number { ap_on_int(x, $1.field, $2); }
    | AP_BOOL_OPTION { ap_on_bool(x, $1.field, !$1.negate); }
    | IDENT { ap_on_posarg(x, $1); }
    | NUMBER { ap_on_posarg(x, mpz_get_str(NULL, 10, $1)); }
    | AP_ERROR
    ;

ident: IDENT { $$ = $1; }
    | ASSIGN IDENT { $$ = $2; }
    ;

number: NUMBER { memcpy($$, $1, sizeof(mpz_t)); }
    | ASSIGN NUMBER { memcpy($$, $2, sizeof(mpz_t)); }
    ;

%%
