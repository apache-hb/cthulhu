%define parse.error verbose
%define api.pure full
%lex-param { void *scanner }
%parse-param { void *scanner } { scanner_t *x }
%locations
%expect 0

%code requires {
    #include "ctu/ast/scanner.h"
    #include "ctu/ast/ast.h"
    #include "ctu/util/str.h"
}

%{
#include <stdio.h>
int yylex();
void yyerror();
%}

%union {
    struct {
        char *text;
        int base;
    } digit;

    char *text;
    node_t *node;
    nodes_t *nodes;
    bool mut;
}

%token<text>
    IDENT "identifier"

%token<digit>
    DIGIT "integer literal"

%token
    ADD "`+`"
    SUB "`-`"
    MUL "`*`"
    DIV "`/`"
    REM "`%`"

    BITAND "`&`"

    GT "`>`"
    GTE "`>=`"
    LT "`<`"
    LTE "`<=`"

    EQ "`==`"
    NEQ "`!=`"

%token
    LPAREN "`(`"
    RPAREN "`)`"
    LBRACE "`{`"
    RBRACE "`}`"

%token
    QUESTION "`?`"
    SEMI "`;`"
    COMMA "`,`"
    COLON "`:`"
    ASSIGN "`=`"

%token
    BOOL_TRUE "`true`"
    BOOL_FALSE "`false`"

%token
    RETURN "`return`"
    DEF "`def`"
    IF "`if`"
    ELSE "`else`"
    AS "`as`"
    FINAL "`final`"
    VAR "`var`"
    WHILE "`while`"
    EXPORTED "`export`"
    END 0 "end of file"

%type<node>
    decl function param variable /* declarations */
    type typename pointer /* types */
    stmt stmts return if else elseif branch assign while /* statements */
    primary postfix unary multiply add compare equality expr /* expressions */

%type<nodes>
    stmtlist
    args arglist
    params paramlist

%type<mut>
    mut

%start file

%%

/**
 * toplevel decls
 */

file: unit END ;

unit: decl { x->ast = ast_list($1); }
    | unit decl { ast_append(x->ast, $2); }
    ;

decl: function { $$ = $1; }
    | EXPORTED function { $$ = make_exported($2); }
    ;

function: DEF IDENT params COLON type stmts { 
        $$ = ast_decl_func(x, merge_locations(@1, @2), 
            /* name */ $2, 
            /* params */ $3,
            /* result */ $5,
            /* body */ $6
        ); 
    }
    ;

variable: mut IDENT ASSIGN expr SEMI { $$ = ast_decl_var(x, @$, $1, $2, $4); }
    ;

mut: VAR { $$ = true; }
    | FINAL { $$ = false; }
    ;

params: LPAREN RPAREN { $$ = ast_list(NULL); }
    | LPAREN paramlist RPAREN { $$ = $2; }
    ;

paramlist: param { $$ = ast_list($1); }
    | paramlist COMMA param { $$ = ast_append($1, $3); }
    ;

param: IDENT COLON type { $$ = ast_decl_param(x, @$, $1, $3); }
    ;

/**
 * statements
 */

stmtlist: %empty { $$ = ast_list(NULL); }
    | stmtlist stmt { $$ = ast_append($1, $2); }
    ;

stmts: LBRACE stmtlist RBRACE { $$ = ast_stmts(x, @$, $2); }
    ;

return: RETURN SEMI { $$ = ast_return(x, @$, NULL); }
    | RETURN expr SEMI { $$ = ast_return(x, @$, $2); }
    ;

branch: if { $$ = $1; }
    | if else { $$ = add_branch($1, $2); }
    ;

if: IF expr stmts { $$ = ast_branch(x, @$, $2, $3); }
    ;

else: ELSE stmts { $$ = ast_branch(x, @$, NULL, $2); }
    | elseif else { $$ = add_branch($1, $2); }
    ;

elseif: ELSE IF expr stmts { $$ = ast_branch(x, @$, $3, $4); }
    ;

assign: expr ASSIGN expr SEMI { $$ = ast_assign(x, @$, $1, $3); }
    ;

while: WHILE expr stmts { $$ = ast_while(x, @$, $2, $3); }
    ;

stmt: expr SEMI { $$ = $1; }
    | return { $$ = $1; }
    | branch { $$ = $1; }
    | stmts { $$ = $1; }
    | variable { $$ = $1; }
    | assign { $$ = $1; }
    | while { $$ = $1; }
    ;

/**
 * types
 */

type: typename { $$ = $1; }
    | pointer { $$ = $1; }
    | VAR LPAREN type RPAREN { $$ = ast_mut(x, @$, $3); }
    ;

pointer: MUL type { $$ = ast_pointer(x, @$, $2); }
    ;

typename: IDENT { $$ = ast_symbol(x, @$, strdup($1)); }
    ;

/**
 * expressions
 */


args: %empty { $$ = ast_list(NULL); }
    | arglist { $$ = $1; }
    ;

arglist: expr { $$ = ast_list($1); }
    | arglist COMMA expr { $$ = ast_append($1, $3); }
    ;


primary: LPAREN expr RPAREN { $$ = $2; }
    | typename { $$ = $1; }
    | DIGIT { $$ = ast_digit(x, @$, $1.text, $1.base); }
    | BOOL_TRUE { $$ = ast_bool(x, @$, true); }
    | BOOL_FALSE { $$ = ast_bool(x, @$, false); }
    ;

postfix: primary { $$ = $1; }
    | postfix QUESTION { $$ = ast_unary(x, @$, UNARY_TRY, $1); }
    | postfix LPAREN args RPAREN { $$ = ast_call(x, @$, $1, $3); }
    | postfix AS type { $$ = ast_cast(x, @$, $1, $3); }
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = ast_unary(x, @$, UNARY_ABS, $2); }
    | SUB unary { $$ = ast_unary(x, @$, UNARY_NEG, $2); }
    | BITAND unary { $$ = ast_unary(x, @$, UNARY_REF, $2); }
    | MUL unary { $$ = ast_unary(x, @$, UNARY_DEREF, $2); }
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

compare: add { $$ = $1; }
    | compare GT add { $$ = ast_binary(x, @$, BINARY_GT, $1, $3); }
    | compare GTE add { $$ = ast_binary(x, @$, BINARY_GTE, $1, $3); }
    | compare LT add { $$ = ast_binary(x, @$, BINARY_LT, $1, $3); }
    | compare LTE add { $$ = ast_binary(x, @$, BINARY_LTE, $1, $3); }
    ;

equality: compare { $$ = $1; }
    | equality EQ compare { $$ = ast_binary(x, @$, BINARY_EQ, $1, $3); }
    | equality NEQ compare{ $$ = ast_binary(x, @$, BINARY_NEQ, $1, $3); }
    ;

expr: equality { $$ = $1; }
    ;
    
%%
