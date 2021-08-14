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
    block init ident number

%type<vector>
    consts inits
    vars names

%token
    DOT "`.`"
    SEMICOLON "`;`"
    EQUALS "`=`"
    COMMA "`,`"
    END 0 

%token
    CONST "const"
    VAR "var"

%start program

%%

program: block DOT { scan_export(x, $1); }
    ;

block: consts vars { $$ = pl0_program(x, @$, $1, $2); }
    ;

consts: %empty { $$ = vector_new(0); }
    | CONST inits SEMICOLON { $$ = $2; }
    ;

inits: init { $$ = vector_init($1); }
    | inits COMMA init { vector_push(&$1, $3); $$ = $1; }
    ;

init: ident EQUALS number { $$ = pl0_const(x, @$, $1, $3); }
    ;

vars: %empty { $$ = vector_new(0); }
    | VAR names SEMICOLON { $$ = $2; }
    ;

names: ident { $$ = vector_init($1); }
    | names COMMA ident { vector_push(&$1, $3); $$ = $1; }
    ;

ident: IDENT { $$ = pl0_ident(x, @$, $1); }
    ;

number: NUMBER { $$ = pl0_number(x, @$, $1); }
    ;

%%
