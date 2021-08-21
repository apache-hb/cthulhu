%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ctu}

%code requires {
    #define YYSTYPE CTUSTYPE
    #define YYLTYPE CTULTYPE
    
    #include "scan.h"
    #include "ast.h"
}

%{
#include "scan.h"
int ctulex();
void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    node_t *node;

    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"

%token
    END 0 

%type<node>
    unit

%start program

%%

program: unit END { scan_export(x, $1); }
    ;

unit: %empty { $$ = NULL; }
    ;

%%