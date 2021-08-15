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
    node_t *node;
    pl0_node_t *pl0node;
    vector_t *vector;

    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"

%token<number>
    NUMBER "number"

%type<node>
    ident number

%type<pl0node>
    block init procedure
    statement statements call

%type<vector>
    consts inits
    vars names
    procedures
    stmtlist
    proclist

%token
    DOT "`.`"
    SEMICOLON "`;`"
    EQUALS "`=`"
    COMMA "`,`"

%token
    CONST "const"
    VAR "var"
    PROCEDURE "procedure"

    START "begin"
    END "end"

    IF "if"
    THEN "then"
    WHILE "while"
    DO "do"
    CALL "call"

%start program

%%

program: block DOT { scan_export(x, $1); }
    ;

block: consts vars procedures { $$ = pl0_program(x, @$, $1, $2, $3); }
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

procedures: %empty { $$ = vector_new(0); }
    | proclist { $$ = $1; }
    ;

proclist: procedure { $$ = vector_init($1); }
    | proclist procedure { vector_push(&$1, $2); $$ = $1; }
    ;

procedure: PROCEDURE ident SEMICOLON vars statements { $$ = pl0_procedure(x, @$, $2, $4); }
    ;

statement: statements { $$ = $1; }
    | call { $$ = $1; }
    ;

call: CALL ident { $$ = pl0_call(x, @$, $2); }
    ;

statements: START stmtlist END { $$ = pl0_statements(x, @$, $2); }
    ;

stmtlist: statement { $$ = vector_init($1); }
    | stmtlist SEMICOLON statement { vector_push(&$1, $3); $$ = $1; }
    ;

ident: IDENT { $$ = ast_ident(x, @$, $1); }
    ;

number: NUMBER { $$ = ast_digit(x, @$, $1); }
    ;

%%
