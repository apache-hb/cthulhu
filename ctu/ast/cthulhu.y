%define parse.error verbose
%define api.pure full
%lex-param { void *scanner }
%parse-param { void *scanner } { scanner_t *x }
%locations
%expect 0

%code requires {
    #include "ctu/ast/scanner.h"
    #include "ctu/ast/ast.h"
}

%{
#include <stdio.h>
int yylex();
void yyerror();
%}

%union {
    char *text;
    node_t *node;
    nodes_t *nodes;
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
    LBRACE "`{`"
    RBRACE "`}`"

%token
    QUESTION "`?`"
    SEMI "`;`"

%token
    RETURN "`return`"
    DEF "`def`"
    IF "`if`"
    END 0 "end of file"

%type<node>
    decl function /* declarations */
    stmt stmts return if /* statements */
    primary postfix unary multiply add expr /* expressions */

%type<nodes>
    stmtlist

%start file

%%

file: unit END ;

unit: decl { x->ast = ast_list($1); }
    | unit decl { ast_append(x->ast, $2); }
    ;

decl: function { $$ = $1; }
    ;

function: DEF IDENT stmts { $$ = ast_decl_func(x, merge_locations(@1, @2), $2, $3); }
    ;

stmtlist: %empty { $$ = ast_list(NULL); }
    | stmtlist stmt { $$ = ast_append($1, $2); }
    ;

stmts: LBRACE stmtlist RBRACE { $$ = ast_stmts(x, @$, $2); }
    ;

return: RETURN SEMI { $$ = ast_return(x, @$, NULL); }
    | RETURN expr SEMI { $$ = ast_return(x, @$, $2); }
    ;

if: IF expr stmts { $$ = ast_branch(x, @$, $2, $3); }
    ;

stmt: expr SEMI { $$ = $1; }
    | return { $$ = $1; }
    | if { $$ = $1; }
    | stmts { $$ = $1; }
    ;

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
