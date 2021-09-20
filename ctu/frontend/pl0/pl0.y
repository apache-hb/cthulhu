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
    pl0_t *pl0;
    vector_t *vector;

    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"

%token<number>
    NUMBER "number"

%type<pl0>
    number factor term expr math
    unary condition
    block init procedure name
    statement statements toplevel

%type<vector>
    consts inits
    vars names
    procedures
    stmtlist
    proclist

%token
    DOT "`.`"
    SEMICOLON "`;`"
    COMMA "`,`"
    ASSIGN ":="

    EQUALS "`=`"
    ODD "odd"
    NOTEQUAL "#"
    LESS "<"
    GREATER ">"
    LESSEQ "<="
    GREATEQ ">="

    LPAREN "("
    RPAREN ")"

    ADD "+"
    SUB "-"

    MUL "*"
    DIV "/"

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

    PRINT "!"

%start program

%%

program: block DOT { scan_export(x, $1); }
    ;

block: consts vars procedures toplevel { $$ = pl0_module(x, @$, $1, $2, $3, $4); }
    ;

toplevel: %empty { $$ = NULL; }
    | statement { $$ = $1; }
    ;

consts: %empty { $$ = vector_new(0); }
    | CONST inits SEMICOLON { $$ = $2; }
    ;

inits: init { $$ = vector_init($1); }
    | inits COMMA init { vector_push(&$1, $3); $$ = $1; }
    ;

init: IDENT EQUALS expr { $$ = pl0_value(x, @$, $1, $3); }
    ;

vars: %empty { $$ = vector_new(0); }
    | VAR names SEMICOLON { $$ = $2; }
    ;

names: name { $$ = vector_init($1); }
    | names COMMA name { vector_push(&$1, $3); $$ = $1; }
    ;

name: IDENT { $$ = pl0_value(x, @$, $1, NULL); }
    ;

procedures: %empty { $$ = vector_new(0); }
    | proclist { $$ = $1; }
    ;

proclist: procedure { $$ = vector_init($1); }
    | proclist procedure { vector_push(&$1, $2); $$ = $1; }
    ;

procedure: PROCEDURE IDENT SEMICOLON vars START stmtlist END SEMICOLON { $$ = pl0_procedure(x, @$, $2, $4, $6, false); }
    ;

statement: statements { $$ = $1; }
    | CALL IDENT { $$ = pl0_call(x, @$, $2); }
    | IDENT ASSIGN expr { $$ = pl0_assign(x, @$, $1, $3); }
    | IF condition THEN statement { $$ = pl0_branch(x, @$, $2, $4); }
    | WHILE condition DO statement { $$ = pl0_loop(x, @$, $2, $4); }
    | PRINT expr { $$ = pl0_print(x, @$, $2); }
    ;

statements: START stmtlist END { $$ = pl0_stmts(x, @$, $2); }
    ;

stmtlist: statement { $$ = vector_init($1); }
    | stmtlist SEMICOLON statement { vector_push(&$1, $3); $$ = $1; }
    ;

factor: IDENT { $$ = pl0_ident(x, @$, $1); }
    | number { $$ = $1; }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

term: factor { $$ = $1; }
    | factor MUL term { $$ = pl0_binary(x, @$, BINARY_MUL, $1, $3); }
    | factor DIV term { $$ = pl0_binary(x, @$, BINARY_DIV, $1, $3); }
    ;

math: term { $$ = $1; }
    | term ADD math { $$ = pl0_binary(x, @$, BINARY_ADD, $1, $3); }
    | term SUB math { $$ = pl0_binary(x, @$, BINARY_SUB, $1, $3); }
    ;

unary: math { $$ = $1; }
    | SUB unary { $$ = pl0_unary(x, @$, UNARY_NEG, $2); }
    | ADD unary { $$ = pl0_unary(x, @$, UNARY_ABS, $2); }
    ;

expr: unary { $$ = $1; }
    ;

condition: ODD expr { $$ = pl0_odd(x, @$, $2); }
    | expr EQUALS expr { $$ = pl0_binary(x, @$, BINARY_EQ, $1, $3); }
    | expr NOTEQUAL expr { $$ = pl0_binary(x, @$, BINARY_NEQ, $1, $3); }
    | expr LESS expr { $$ = pl0_binary(x, @$, BINARY_LT, $1, $3); }
    | expr LESSEQ expr { $$ = pl0_binary(x, @$, BINARY_LTE, $1, $3); }
    | expr GREATER expr { $$ = pl0_binary(x, @$, BINARY_GT, $1, $3); }
    | expr GREATEQ expr { $$ = pl0_binary(x, @$, BINARY_GTE, $1, $3); }
    ;

number: NUMBER { $$ = pl0_digit(x, @$, $1); }
    ;

%%
