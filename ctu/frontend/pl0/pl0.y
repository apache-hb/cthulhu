%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {pl0}

%code requires {
    #define YYSTYPE PL0STYPE
    #define YYLTYPE PL0LTYPE
    
    #include "scan.h"
    #include "ast.h"
}

%{
#include "scan.h"
int pl0lex();
void pl0error(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    pl0_node_t *node;
    vector_t *vector;

    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"

%token<number>
    NUMBER "number"

%type<node>
    block init

%type<vector>
    inits consts

%token
    DOT "`.`"
    SEMICOLON "`;`"
    EQUALS "`=`"
    COMMA "`,`"
    END 0 

%token
    CONST "const"

%start program

%%

program: block DOT { scan_export(x, $1); }
    ;

block: consts { $$ = pl0_program(x, @$, $1); }
    ;

consts: %empty { $$ = vector_new(0); }
    | CONST inits SEMICOLON { $$ = $2; }
    ;

inits: init { $$ = vector_init($1); }
    | inits COMMA init { vector_push(&$1, $3); $$ = $1; }
    ;

init: IDENT EQUALS NUMBER { $$ = pl0_const(x, @$, $1, $3); }
    ;

%%
