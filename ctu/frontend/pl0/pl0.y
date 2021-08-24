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
    vector_t *vector;

    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"

%token<number>
    NUMBER "number"

%type<node>
    ident number factor term expr
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

init: ident EQUALS number { $$ = ast_value(x, @$, $1, NULL, $3); }
    ;

vars: %empty { $$ = vector_new(0); }
    | VAR names SEMICOLON { $$ = $2; }
    ;

names: name { $$ = vector_init($1); }
    | names COMMA name { vector_push(&$1, $3); $$ = $1; }
    ;

name: ident { $$ = ast_value(x, @$, $1, NULL, NULL); }
    ;

procedures: %empty { $$ = vector_new(0); }
    | proclist { $$ = $1; }
    ;

proclist: procedure { $$ = vector_init($1); }
    | proclist procedure { vector_push(&$1, $2); $$ = $1; }
    ;

procedure: PROCEDURE ident SEMICOLON vars statements SEMICOLON { $$ = pl0_procedure(x, @$, $2, $4, $5); }
    ;

statement: statements { $$ = $1; }
    | CALL ident { $$ = ast_call(x, @$, $2, vector_new(0)); }
    | ident ASSIGN expr { $$ = ast_assign(x, @$, $1, $3); }
    | IF condition THEN statement { $$ = ast_branch(x, @$, $2, $4, NULL); }
    | WHILE condition DO statement { $$ = ast_while(x, @$, $2, $4); }
    ;

statements: START stmtlist END { $$ = ast_stmts(x, @$, $2); }
    ;

stmtlist: statement { $$ = vector_init($1); }
    | stmtlist SEMICOLON statement { vector_push(&$1, $3); $$ = $1; }
    ;

factor: ident { $$ = $1; }
    | number { $$ = $1; }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

term: factor { $$ = $1; }
    | factor MUL term { $$ = ast_binary(x, @$, BINARY_MUL, $1, $3); }
    | factor DIV term { $$ = ast_binary(x, @$, BINARY_DIV, $1, $3); }
    ;

unary: term { $$ = $1; }
    | SUB unary { $$ = ast_unary(x, @$, UNARY_NEG, $2); }
    | ADD unary { $$ = ast_unary(x, @$, UNARY_ABS, $2); }
    ;

expr: unary { $$ = $1; }
    ;

condition: ODD expr { $$ = pl0_odd(x, @$, $2); }
    | expr EQUALS expr { $$ = ast_binary(x, @$, BINARY_EQ, $1, $3); }
    | expr NOTEQUAL expr { $$ = ast_binary(x, @$, BINARY_NEQ, $1, $3); }
    | expr LESS expr { $$ = ast_binary(x, @$, BINARY_LT, $1, $3); }
    | expr LESSEQ expr { $$ = ast_binary(x, @$, BINARY_LTE, $1, $3); }
    | expr GREATER expr { $$ = ast_binary(x, @$, BINARY_GT, $1, $3); }
    | expr GREATEQ expr { $$ = ast_binary(x, @$, BINARY_GTE, $1, $3); }
    ;

ident: IDENT { $$ = ast_ident(x, @$, $1); }
    ;

number: NUMBER { $$ = ast_digit(x, @$, $1); }
    ;

%%