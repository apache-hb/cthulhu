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

#define LOC(n, l) n->loc = l

%}

%union {
    struct {
        char *text;
        int base;
    } digit;
    char *text;
    node_t *node;
    nodes_t *nodes;
}

%token<text>
    IDENT "identifier"

%token<digit>
    DIGIT "integer literal"

%token
    ADD "`+`"
    SUB "`-`"
    DIV "`/`"
    MUL "`*`"
    REM "`%`"

%token
    SEMI "`;`"
    QUESTION "`?`"
    COLON "`:`"
    ASSIGN "`=`"

%token
    LPAREN "`(`"
    RPAREN "`)`"
    LBRACE "`{`"
    RBRACE "`}`"

%token
    FINAL "`final`"
    DEF "`def`"
    RETURN "`return`"
    BTRUE "`true`"
    BFALSE "`false`"

%type<node>
    funcdecl /* decls */
    stmt var /* stmts */
    type /* types */
    primary expr additive multiplicative unary ternary postfix /* exprs */

%type<nodes>
    unit stmts

%start unit

%%

unit: funcdecl { x->ast = ast_list($1); }
    | unit funcdecl { x->ast = ast_append(x->ast, $2); }
    ;

funcdecl: DEF IDENT[n] COLON type[r] LBRACE stmts[b] RBRACE { $$ = ast_func(x, @$, $n, $r, ast_stmts(x, @4, $b)); }
    ;

type: IDENT { $$ = ast_typename(x, @$, $1); }
    ;

stmts: %empty { $$ = ast_empty(); }
    | stmts stmt { $$ = ast_append($1, $2); }
    ;

stmt: expr SEMI { $$ = $1; }
    | LBRACE stmts RBRACE { $$ = ast_stmts(x, @$, $2); }
    | RETURN SEMI { $$ = ast_return(x, @$, NULL); }
    | RETURN expr SEMI { $$ = ast_return(x, @$, $2); }
    | var { $$ = $1; }
    ;

var: FINAL IDENT[n] ASSIGN expr[v] SEMI { $$ = ast_var(x, @$, $n, $v); }
    ;

primary: LPAREN expr RPAREN { $$ = $2; }
    | DIGIT { $$ = ast_digit(x, @$, $1.text, $1.base); }
    | IDENT { $$ = ast_ident(x, @$, $1); }
    | BTRUE { $$ = ast_bool(x, @$, true); }
    | BFALSE { $$ = ast_bool(x, @$, false); }
    ;

postfix: primary { $$ = $1; }
    | postfix LPAREN RPAREN { $$ = ast_call(x, @$, $1); }
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = ast_unary(x, @$, $2, ADD); }
    | SUB unary { $$ = ast_unary(x, @$, $2, SUB); }
    ;

multiplicative: unary { $$ = $1; }
    | multiplicative MUL unary { $$ = ast_binary(x, @$, $1, $3, MUL); }
    | multiplicative DIV unary { $$ = ast_binary(x, @$, $1, $3, DIV); }
    | multiplicative REM unary { $$ = ast_binary(x, @$, $1, $3, REM); }
    ;

additive: multiplicative { $$ = $1; }
    | additive ADD multiplicative { $$ = ast_binary(x, @$, $1, $3, ADD); }
    | additive SUB multiplicative { $$ = ast_binary(x, @$, $1, $3, SUB); }
    ;

ternary: additive { $$ = $1; }
    | additive QUESTION expr COLON expr { $$ = ast_ternary(x, @$, $1, $3, $5); }
    ;

expr: ternary { $$ = $1; }
    ;

%%
