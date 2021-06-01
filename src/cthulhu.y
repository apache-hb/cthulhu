%output "bison.c"
%defines "bison.h"

%define parse.error verbose
%define api.pure full
%lex-param { void *scanner }
%parse-param { void *scanner } { scanner_t *x }
%locations
%expect 0

%code requires {
#include "scanner.h"
#include "ast.h"
}

%{

#include <stdio.h>

int yylex();
int yyerror();

%}

%union {
    char *text;
    node_t *node;
}

%token<text>
    IDENT "identifier"
    DIGIT "integer literal"

%token
    ADD "+"
    SUB "-"
    DIV "/"
    MUL "*"
    REM "%"

%token
    SEMI ";"

%token
    LPAREN "("
    RPAREN ")"

%type<node>
    primary expr additive multiplicative unary

%start unit

%%

unit: expr SEMI { x->ast = $1; }
    ;

primary: LPAREN expr RPAREN { $$ = $2; }
    | DIGIT { $$ = ast_digit($1); }
    ;

unary: primary { $$ = $1; }
    | ADD unary { $$ = ast_unary($2, ADD); }
    | SUB unary { $$ = ast_unary($2, SUB); }
    ;

multiplicative: unary { $$ = $1; }
    | multiplicative MUL unary { $$ = ast_binary($1, $3, MUL); }
    | multiplicative DIV unary { $$ = ast_binary($1, $3, DIV); }
    | multiplicative REM unary { $$ = ast_binary($1, $3, REM); }
    ;

additive: multiplicative { $$ = $1; }
    | additive ADD multiplicative { $$ = ast_binary($1, $3, ADD); }
    | additive SUB multiplicative { $$ = ast_binary($1, $3, SUB); }
    ;

expr: additive { $$ = $1; }
    ;

%%
