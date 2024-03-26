%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.location.type {where_t}
%define api.prefix {pl0}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"
    #include "std/vector.h"
    #include "cthulhu/broker/scan.h"
    #include "scan/scan.h"
}

%code requires {
    #include "pl0/ast.h"

    #define YYSTYPE PL0STYPE
    #define YYLTYPE PL0LTYPE
}

%{
int pl0lex(void *lval, void *loc, scan_t *scan);
void pl0error(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    pl0_t *pl0;
    vector_t *vector;
    const vector_t *cvector;

    char *ident;
    mpz_t number;
}

%token<ident>
    IDENT "identifier"

%token<number>
    NUMBER "number"

%type<pl0>
    number factor term expr math mul ident
    unary condition
    block init procedure name
    statement statements toplevel
    import

%type<vector>
    consts inits
    vars names
    procedures
    stmtlist
    proclist
    imports
    paths
    path

%type<cvector>
    module

%token
    DOT "`.`"
    SEMICOLON "`;`"
    COMMA "`,`"
    ASSIGN ":="

    EQUALS "`=`"
    ODD "odd"
    NOTEQUAL "!="
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
    MODULE "module"
    IMPORT "import"
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

program: block DOT { scan_set(x, $1); }
    ;

block: module imports consts vars procedures toplevel { $$ = pl0_module(x, @$, $1, $2, $3, $4, $5, $6); }
    ;

imports: %empty { $$ = ctx_vector_new(0, x); }
    | IMPORT paths SEMICOLON { $$ = $2; }
    ;

paths: import { $$ = ctx_vector_init($1, x); }
    | paths COMMA import { vector_push(&$1, $3); $$ = $1; }
    ;

import: path { $$ = pl0_import(x, @$, $1); }
    ;

path: IDENT { $$ = ctx_vector_init($1, x); }
    | path DOT IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

module: %empty { $$ = &kEmptyVector; }
    | MODULE path SEMICOLON { $$ = $2; }
    ;

toplevel: %empty { $$ = NULL; }
    | stmtlist { $$ = pl0_stmts(x, @$, $1); }
    ;

consts: %empty { $$ = ctx_vector_new(0, x); }
    | CONST inits SEMICOLON { $$ = $2; }
    ;

inits: init { $$ = ctx_vector_init($1, x); }
    | inits COMMA init { vector_push(&$1, $3); $$ = $1; }
    ;

init: IDENT EQUALS expr { $$ = pl0_value(x, @$, $1, $3); }
    ;

vars: %empty { $$ = ctx_vector_new(0, x); }
    | VAR names SEMICOLON { $$ = $2; }
    ;

names: name { $$ = ctx_vector_init($1, x); }
    | names COMMA name { vector_push(&$1, $3); $$ = $1; }
    ;

name: IDENT { $$ = pl0_value(x, @$, $1, NULL); }
    ;

procedures: %empty { $$ = ctx_vector_new(0, x); }
    | proclist { $$ = $1; }
    ;

proclist: procedure { $$ = ctx_vector_init($1, x); }
    | proclist procedure { vector_push(&$1, $2); $$ = $1; }
    ;

procedure: PROCEDURE IDENT SEMICOLON vars START stmtlist END SEMICOLON { $$ = pl0_procedure(x, @$, $2, $4, $6); }
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

stmtlist: statement { $$ = ctx_vector_init($1, x); }
    | stmtlist SEMICOLON statement { vector_push(&$1, $3); $$ = $1; }
    ;

ident: IDENT { $$ = pl0_ident(x, @$, $1); }
    ;

factor: ident { $$ = $1; }
    | number { $$ = $1; }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

term: factor { $$ = $1; }
    | factor DIV term { $$ = pl0_binary(x, @$, eBinaryDiv, $1, $3); }
    ;

mul: term { $$ = $1; }
    | term MUL mul { $$ = pl0_binary(x, @$, eBinaryMul, $1, $3); }
    ;

math: mul { $$ = $1; }
    | mul ADD math { $$ = pl0_binary(x, @$, eBinaryAdd, $1, $3); }
    | mul SUB math { $$ = pl0_binary(x, @$, eBinarySub, $1, $3); }
    ;

unary: math { $$ = $1; }
    | SUB unary { $$ = pl0_unary(x, @$, eUnaryNeg, $2); }
    | ADD unary { $$ = pl0_unary(x, @$, eUnaryAbs, $2); }
    ;

expr: unary { $$ = $1; }
    ;

condition: ODD expr { $$ = pl0_odd(x, @$, $2); }
    | expr EQUALS expr { $$ = pl0_compare(x, @$, eCompareEq, $1, $3); }
    | expr NOTEQUAL expr { $$ = pl0_compare(x, @$, eCompareNeq, $1, $3); }
    | expr LESS expr { $$ = pl0_compare(x, @$, eCompareLt, $1, $3); }
    | expr LESSEQ expr { $$ = pl0_compare(x, @$, eCompareLte, $1, $3); }
    | expr GREATER expr { $$ = pl0_compare(x, @$, eCompareGt, $1, $3); }
    | expr GREATEQ expr { $$ = pl0_compare(x, @$, eCompareGte, $1, $3); }
    ;

number: NUMBER { $$ = pl0_digit(x, @$, $1); }
    ;

%%
