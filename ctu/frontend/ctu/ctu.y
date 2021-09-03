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
    ctu_t *ctu;
    vector_t *vector;

    char *ident;
    mpz_t digit;
}

%token<ident>
    IDENT "identifier"

%token<digit>
    DIGIT "integer literal"

%token
    VAR "`var`"
    SEMI "`;`"
    ASSIGN "`=`"
    END 0 

%type<ctu>
    value decl
    expr primary

%type<vector>
    unit

%start program

%%

program: unit END { scan_export(x, ctu_module(x, @$, $1)); }
    ;

unit: decl { $$ = vector_init($1); }
    | unit decl { vector_push(&$1, $2); $$ = $1; }
    ;

decl: value { $$ = $1; }
    ;

value: VAR IDENT ASSIGN expr SEMI { $$ = ctu_value(x, @$, $2, $4); }
    ;

primary: DIGIT { $$ = ctu_digit(x, @$, $1); }
    | IDENT { $$ = ctu_ident(x, @$, $1); }
    ;

expr: primary { $$ = $1; }
    ;

%%