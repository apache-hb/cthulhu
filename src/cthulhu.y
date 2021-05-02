%output "bison.c"
%defines "bison.h"

%define parse.error verbose // these messages are still awful but better than nothing
%lex-param { void *scanner }
%parse-param { void *scanner }
%define api.pure full

%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "ast.h"

int yylex();
int yyerror();

%}

%union {
    char *text;
    struct node_t *node;
}

/* tokens from flex */
%token<text>
    DIGIT
    IDENT

/* keywords */
%token
    DEF "def"

/* math ops */
%token 
    SEMI ";"
    ADD "+"
    SUB "-"
    MUL "*"
    DIV "/"
    REM "%"

/* language ops */
%token
    LPAREN "("
    RPAREN ")"
    LBRACE "{"
    RBRACE "}"
    COMMA ","
    QUESTION "?"
    COLON ":"
    DOT "."
    END

%type<node> primary call postfix unary multiplicative additive conditional expr

%start unit

%%

unit: decls END
    ;

decls: decl
    | decl decls
    ;

decl: func
    ;

func: DEF IDENT stmt
    ;

stmt: LBRACE stmts RBRACE
    | expr SEMI { dump_node($1); printf("\n"); }
    ;

stmts: %empty
    | stmt stmts
    ;

primary: DIGIT { $$ = new_digit($1); }
    | LPAREN expr[it] RPAREN { $$ = $it; }
    ;

call: postfix LPAREN RPAREN
    | postfix LPAREN exprs RPAREN
    ;

postfix: primary { $$ = $1; }
    //| postfix DOT IDENT
    | call
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = new_unary(UNARY_ABS, $2); }
    | SUB unary { $$ = new_unary(UNARY_NEG, $2); }
    ;

multiplicative: unary { $$ = $1; }
    | multiplicative[lhs] MUL unary[rhs] { $$ = new_binary(BINARY_MUL, $lhs, $rhs); }
    | multiplicative[lhs] DIV unary[rhs] { $$ = new_binary(BINARY_DIV, $lhs, $rhs); }
    | multiplicative[lhs] REM unary[rhs] { $$ = new_binary(BINARY_REM, $lhs, $rhs); }
    ;

additive: multiplicative { $$ = $1; }
    | additive[lhs] ADD multiplicative[rhs] { $$ = new_binary(BINARY_ADD, $lhs, $rhs); }
    | additive[lhs] SUB multiplicative[rhs] { $$ = new_binary(BINARY_SUB, $lhs, $rhs); }
    ;

conditional: additive { $$ = $1; }
    | additive[cond] QUESTION expr[yes] COLON expr[no] { $$ = new_ternary($cond, $yes, $no); }
    ;

expr: conditional { $$ = $1; }
    ;

exprs: expr
    | expr COMMA exprs
    ;

%%

/**

tightest-loosest binding

. ->
() []
unary + - ~ !
?:
< <= > >=
| & ^
<< >>
/ * %
+ -

*/
