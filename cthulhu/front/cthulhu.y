%output "bison.c"
%defines "bison.h"

%define parse.error verbose
%define api.pure full
%lex-param { void *scanner }
%parse-param { void *scanner } { scanner_t *x }
%locations
%expect 0

%code requires {
#include "cthulhu/front/scanner.h"
#include "cthulhu/front/front.h"
}

%{

#include <stdio.h>

int yylex();
int yyerror();

%}

%union {
    char *text;
    node_t *node;
    nodes_t *nodes;
}

%token<text>
    DIGIT "integer literal"

%token
    ADD "+"
    SUB "-"
    DIV "/"
    MUL "*"
    REM "%"

%token
    SEMI ";"
    QUESTION "?"
    COLON ":"

%token
    LPAREN "("
    RPAREN ")"

%type<node>
    primary expr additive multiplicative unary ternary postfix

%type<nodes>
    unit

%start unit

%%

unit: expr SEMI { x->ast = ast_list($1); }
    | unit expr SEMI { x->ast = ast_append(x->ast, $2); }
    ;

primary: LPAREN expr RPAREN { $$ = $2; }
    | DIGIT { $$ = ast_digit($1); }
    ;

postfix: primary { $$ = $1; }
    ;

unary: postfix { $$ = $1; }
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

ternary: additive { $$ = $1; }
    | additive QUESTION expr COLON expr { $$ = ast_ternary($1, $3, $5); }
    ;

expr: ternary { $$ = $1; }
    ;

%%
