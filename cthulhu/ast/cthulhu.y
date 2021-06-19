%define parse.error verbose
%define api.pure full
%define api.prefix {ctu_}
%lex-param { void *scanner }
%parse-param { void *scanner } { scanner_t *x }
%locations
%expect 0

%{
#include "cthulhu/ast/scanner.h"
#include "cthulhu/ast/ast.h"

int ctu_lex();
int ctu_error();
%}

%union {
    char *text;
    node_t *node;
}

%token<text>
    IDENT "identifier"
    DIGIT "integer literal"

%token
    ADD "`+`"
    SUB "`-`"
    MUL "`*`"
    DIV "`/`"
    REM "`%`"

%token
    LPAREN "`(`"
    RPAREN "`)`"

%token
    QUESTION "`?`"

%type<node>
    primary postfix unary multiply add expr

%start expr

%%

primary: LPAREN expr RPAREN { $$ = $2; }
    | IDENT { $$ = ast_ident(x, @$, $1); }
    | DIGIT { $$ = ast_digit(x, @$, $1); }
    ;

postfix: primary { $$ = $1; }
    | postfix QUESTION { $$ = ast_unary(x, @$, UNARY_TRY, $1); }
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = ast_unary(x, @$, UNARY_ABS, $2); }
    | SUB unary { $$ = ast_unary(x, @$, UNARY_NEG, $2); }
    ;

multiply: unary { $$ = $1; }
    | multiply MUL unary { $$ = ast_binary(x, @$, BINARY_MUL, $1, $3); }
    | multiply DIV unary { $$ = ast_binary(x, @$, BINARY_DIV, $1, $3); }
    | multiply REM unary { $$ = ast_binary(x, @$, BINARY_REM, $1, $3); }
    ;

add: multiply { $$ = $1; }
    | add ADD multiply { $$ = ast_binary(x, @$, BINARY_ADD, $1, $3); }
    | add SUB multiply { $$ = ast_binary(x, @$, BINARY_SUB, $1, $3); }
    ;

expr: add { $$ = $1; }
    ;
    
%%
