%output "bison.c"
%defines "bison.h"

%define parse.error verbose // these messages are still awful but better than nothing
%define api.pure full
%lex-param { void *scanner }
%parse-param { void *scanner } { struct node_t **node }
%locations
%expect 0 // TODO: resolve dangling else without requiring compound stmts everywhere

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
    struct nodes_t *nodes;
    int cond;
}

/* tokens from flex */
%token<text>
    DIGIT "integer literal"
    IDENT "identifier"
    STRING "string literal"

/* keywords */
%token
    DEF "def"
    VAR "var"
    RETURN "return"
    EXPORT "export"
    RECORD "record"
    FINAL "final"
    BOOL_TRUE "true"
    BOOL_FALSE "false"
    IF "if"
    ELSE "else"
    WHILE "while"
    BREAK "break"
    CONTINUE "continue"

/* math ops */
%token 
    SEMI ";"
    ADD "+"
    SUB "-"
    MUL "*"
    DIV "/"
    REM "%"
    BITAND "&"
    NOT "!"

/* language ops */
%token
    LPAREN "("
    RPAREN ")"
    LBRACE "{"
    RBRACE "}"
    COMMA ","
    QUESTION "?"
    COLON ":"
    ASSIGN "="
    ARROW "->"
    DOT "."
    END 0 "end of file"

%type<node> 
    primary call postfix unary multiplicative additive conditional expr 
    decl func stmt type param result var compound declb init
    cond if else record field while

%type<cond>
    mut

%type<nodes>
    exprs stmts decls params fparams types fields

%right LBRACE ELSE

%start unit

%%

unit: decls END { *node = new_compound($1); }
    ;

decls: decl { $$ = new_node_list($1); }
    | decls decl { $$ = node_append($1, $2); }
    ;

decl: declb { $$ = $1; }
    | EXPORT declb { $$ = set_exported($2, 1); }
    ;

declb: func { $$ = $1; }
    | var { $$ = $1; }
    | record { $$ = $1; }
    ;

record: RECORD IDENT LBRACE fields RBRACE { $$ = new_record($2, $4); }
    ;

fields: field { $$ = new_node_list($1); }
    | fields field { $$ = node_append($1, $2); }
    ;

field: IDENT COLON type { $$ = new_param($1, $3); }
    ;

var: mut[m] IDENT[name] result[it] init[i] SEMI { $$ = new_var($name, $it, $i, $m); }
    ;

init: %empty { $$ = NULL; }
    | ASSIGN expr { $$ = $2; }
    ;

mut: VAR { $$ = 1; }
    | FINAL { $$ = 0; }
    ;

func: DEF IDENT[name] fparams[args] result[res] compound[body] { $$ = new_func($name, $args, $res, $body); }
    ;

result: %empty { $$ = NULL; } 
    | COLON type { $$ = $2; }
    ;

fparams: LPAREN RPAREN { $$ = empty_node_list(); }
    | LPAREN params RPAREN { $$ = $2; }
    ;

params: param { $$ = new_node_list($1); }
    | params COMMA param { $$ = node_append($1, $3); }
    ;

param: IDENT COLON type { $$ = new_param($1, $3); }
    ;

stmt: compound { $$ = $1; }
    | expr ASSIGN expr SEMI { $$ = new_assign($1, $3); }
    | expr SEMI { $$ = $1; }
    | RETURN SEMI { $$ = new_return(NULL); }
    | RETURN expr SEMI { $$ = new_return($2); }
    | var { $$ = $1; }
    | if { $$ = $1; }
    | while { $$ = $1; }
    | BREAK SEMI { $$ = new_break(); }
    | CONTINUE SEMI { $$ = new_continue(); }
    ;

while: WHILE cond compound { $$ = new_while($2, $3); }
    ;

if: IF cond compound else { $$ = new_branch($2, $3, $4); }
    ;

else: ELSE compound { $$ = new_branch(NULL, $2, NULL); }
    | %empty { $$ = NULL; }
    ;

cond: LPAREN expr RPAREN { $$ = $2; }
    ;

compound: LBRACE stmts RBRACE { $$ = new_compound($2); }
    ;

stmts: %empty { $$ = empty_node_list(); }
    | stmts stmt { $$ = node_append($1, $2); }
    ;

primary: DIGIT { $$ = new_digit($1); }
    | BOOL_TRUE { $$ = new_bool(1); }
    | BOOL_FALSE { $$ = new_bool(0); }
    | IDENT { $$ = new_name($1); }
    | LPAREN expr[it] RPAREN { $$ = $it; }
    | STRING { $$ = new_string($1); }
    ;

call: postfix LPAREN RPAREN { $$ = new_call($1, empty_node_list()); }
    | postfix LPAREN exprs[args] RPAREN { $$ = new_call($1, $args); }
    ;

postfix: primary { $$ = $1; }
    | postfix DOT IDENT { $$ = new_access($1, $3); }
    | call { $$ = $1; }
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = new_unary(UNARY_ABS, $2); }
    | SUB unary { $$ = new_unary(UNARY_NEG, $2); }
    | BITAND unary { $$ = new_unary(UNARY_REF, $2); }
    | MUL unary { $$ = new_unary(UNARY_DEREF, $2); }
    | NOT unary { $$ = new_unary(UNARY_NOT, $2); }
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

exprs: expr { $$ = new_node_list($1); }
    | exprs COMMA expr[it] { $$ = node_append($1, $it); }
    ;

type: IDENT { $$ = new_typename($1); }
    | MUL type { $$ = new_pointer($2); }
    | LPAREN types[args] RPAREN ARROW type[res] { $$ = new_closure($args, $res); }
    | LPAREN RPAREN ARROW type[res] { $$ = new_closure(empty_node_list(), $res); }
    ;

types: type { $$ = new_node_list($1); }
    | types COMMA type[it] { $$ = node_append($1, $it); }
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
