%define parse.error verbose
%define api.pure full
%lex-param { void *scanner }
%parse-param { void *scanner } { scanner_t *x }
%locations
%expect 0
%define api.prefix {ct}

%code requires {
    #define YYSTYPE CTSTYPE
    #define YYLTYPE CTLTYPE

    #include "ctu/ast/scanner.h"
    #include "ctu/ast/ast.h"
    #include "ctu/util/str.h"
    #include "ctu/util/report.h"
}

%{
#include <stdio.h>
int ctlex();
void cterror();
%}

%union {
    struct {
        char *text;
        int base;
    } digit;

    struct {
        char *name;
        node_t *node;
    } field;

    char *text;
    node_t *node;
    list_t *nodes;
    attrib_t attrib;
    bool mut;
}

%token<text>
    IDENT "identifier"
    STRING "string literal"

%token<digit>
    DIGIT "integer literal"

%token
    ADD "`+`"
    SUB "`-`"
    MUL "`*`"
    DIV "`/`"
    REM "`%`"

    GT "`>`"
    GTE "`>=`"
    LT "`<`"
    LTE "`<=`"

    EQ "`==`"
    NEQ "`!=`"

    SHL "`<<`"
    SHR "`>>`"
    XOR "`^`"

    AND "`&&`"
    OR "`||`"

    BITAND "`&`"
    BITOR "`|`"

    DOT "`.`"
    ARROW "`->`"

%token
    LPAREN "`(`"
    RPAREN "`)`"
    LBRACE "`{`"
    RBRACE "`}`"
    LSQUARE "`[`"
    RSQUARE "`]`"

%token
    QUESTION "`?`"
    SEMI "`;`"
    COMMA "`,`"
    COLON "`:`"
    COLON2 "`::`"
    ASSIGN "`=`"
    AT "`@`"

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
    BREAK "`break`"
    CONTINUE "`continue`"
    EXPORTED "`export`"
    STRUCT "`struct`"
    UNION "`union`"
    ENUM "`enum`"
    IMPORT "`import`"
    END 0 "end of file"

%type<node>
    decl declbase function param variable struct field /* declarations */
    type typename pointer array /* types */
    stmt stmts return if else elseif branch assign while /* statements */
    primary postfix unary multiply add compare equality or and shift xor bits expr /* expressions */
    import attrib union enum item digit

%type<nodes>
    stmtlist fields enums
    args arglist
    params paramlist
    names imports unit
    decorate decorators

%type<mut>
    mut

%type<attrib>
    attribs

%start file

%%

/**
 * toplevel decls
 */

file: unit END { x->ast = ast_root(x, new_list(NULL), $1); }
    | imports unit END { x->ast = ast_root(x, $1, $2); }
    ;

unit: decl { $$ = new_list($1); }
    | unit decl { list_push($1, $2); }
    ;

decl: decorate attribs declbase { $$ = ast_attribs($3, $2, $1); }
    ;

decorate: %empty { $$ = new_list(NULL); }
    | decorators { $$ = $1; }
    ;

decorators: attrib { $$ = new_list($1); }
    | decorators attrib { $$ = list_push($1, $2); }
    ;

attrib: AT names { $$ = ast_attrib(x, @$, $2, new_list(NULL)); }
    | AT names LPAREN args RPAREN { $$ = ast_attrib(x, @$, $2, $4); }
    ;

attribs: %empty { $$ = 0; }
    | EXPORTED { $$ = ATTR_EXPORT; }
    ;

declbase: function { $$ = $1; }
    | variable { $$ = $1; }
    | struct { $$ = $1; }
    | union { $$ = $1; }
    | enum { $$ = $1; }
    ;

function: DEF IDENT params COLON type stmts { 
        /* function body */
        $$ = ast_decl_func(x, merge_locations(@1, @2), 
            /* name */ $2, 
            /* params */ $3,
            /* result */ $5,
            /* body */ $6
        ); 
    }
    | DEF IDENT params COLON type SEMI {
        /* function predefine */
        $$ = ast_decl_func(x, merge_locations(@1, @2),
            /* name */ $2,
            /* params */ $3,
            /* result */ $5,
            /* body */ NULL
        );
    }
    ;

variable: mut IDENT ASSIGN expr SEMI { $$ = ast_decl_var(x, @$, $1, $2, NULL, $4); }
    | mut IDENT COLON type SEMI { $$ = ast_decl_var(x, @$, $1, $2, $4, NULL); }
    | mut IDENT COLON type ASSIGN expr SEMI { $$ = ast_decl_var(x, @$, $1, $2, $4, $6); }
    ;

struct: STRUCT IDENT LBRACE fields RBRACE { $$ = ast_decl_struct(x, @$, $2, $4); }
    ;

union: UNION IDENT LBRACE fields RBRACE { $$ = ast_decl_union(x, @$, $2, $4); }
    ;

fields: field { $$ = new_list($1); }
    | fields COMMA field { $$ = list_push($1, $3); }
    ;

field: IDENT COLON type { $$ = ast_field(x, @$, $1, $3); }
    ;

enum: ENUM IDENT LBRACE enums RBRACE { $$ = ast_decl_enum(x, @$, $2, $4); }
    ;

enums: item { $$ = new_list($1); }
    | enums COMMA item { $$ = list_push($1, $3); }
    ;

item: IDENT { $$ = ast_enum_item(x, @$, $1, NULL); }
    | IDENT ASSIGN expr { $$ = ast_enum_item(x, @$, $1, $3); }
    ;

mut: VAR { $$ = true; }
    | FINAL { $$ = false; }
    ;

params: LPAREN RPAREN { $$ = new_list(NULL); }
    | LPAREN paramlist RPAREN { $$ = $2; }
    ;

paramlist: param { $$ = new_list($1); }
    | paramlist COMMA param { $$ = list_push($1, $3); }
    ;

param: IDENT COLON type { $$ = ast_decl_param(x, @$, $1, $3); }
    ;

imports: import { $$ = new_list($1); }
    | imports import { $$ = list_push($1, $2); }
    ;

import: IMPORT names SEMI { $$ = ast_import(x, @$, $2); }
    ;

/**
 * statements
 */

stmtlist: %empty { $$ = new_list(NULL); }
    | stmtlist stmt { $$ = list_push($1, $2); }
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
    | BREAK SEMI { $$ = ast_break(x, @$); }
    | CONTINUE SEMI { $$ = ast_continue(x, @$); }
    | error SEMI { $$ = ast_noop(); }
    ;

/**
 * types
 */

type: typename { $$ = $1; }
    | pointer { $$ = $1; }
    | array { $$ = $1; }
    | VAR LPAREN type RPAREN { $$ = ast_mut(x, @$, $3); }
    ;

pointer: MUL type { $$ = ast_pointer(x, @$, $2); }
    ;

typename: names { $$ = ast_symbol(x, @$, $1); }

names: IDENT { $$ = new_list($1); }
    | names COLON2 IDENT { $$ = list_push($1, $3); }
    ;

array: LSQUARE type RSQUARE { $$ = ast_array(x, @$, $2, NULL); }
    | LSQUARE type COLON digit RSQUARE { $$ = ast_array(x, @$, $2, $4); }
    | LSQUARE type COLON error RSQUARE { 
        $$ = ast_array(x, @$, $2, NULL); 
        reportf(LEVEL_ERROR, $$, "array length must be an integer literal");
    }
    ;

/**
 * expressions
 */

args: %empty { $$ = new_list(NULL); }
    | arglist { $$ = $1; }
    ;

arglist: expr { $$ = new_list($1); }
    | arglist COMMA expr { $$ = list_push($1, $3); }
    ;

digit: DIGIT { $$ = ast_digit(x, @$, $1.text, $1.base); }
    ;

primary: LPAREN expr RPAREN { $$ = $2; }
    | typename { $$ = $1; }
    | digit { $$ = $1; }
    | BOOL_TRUE { $$ = ast_bool(x, @$, true); }
    | BOOL_FALSE { $$ = ast_bool(x, @$, false); }
    | STRING { $$ = ast_string(x, @$, $1); }
    ;

postfix: primary { $$ = $1; }
    | postfix QUESTION { $$ = ast_unary(x, @$, UNARY_TRY, $1); }
    | postfix LPAREN args RPAREN { $$ = ast_call(x, @$, $1, $3); }
    | postfix AS type { $$ = ast_cast(x, @$, $1, $3); }
    | postfix DOT IDENT { $$ = ast_access(x, @$, $1, $3, false); }
    | postfix ARROW IDENT { $$ = ast_access(x, @$, $1, $3, true); }
    | postfix LSQUARE expr RSQUARE { $$ = ast_index(x, @$, $1, $3); }
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
    | equality NEQ compare { $$ = ast_binary(x, @$, BINARY_NEQ, $1, $3); }
    ;

shift: equality { $$ = $1; }
    | shift SHL equality { $$ = ast_binary(x, @$, BINARY_SHL, $1, $3); }
    | shift SHR equality { $$ = ast_binary(x, @$, BINARY_SHR, $1, $3); }
    ;

bits: shift { $$ = $1; }
    | bits BITAND shift { $$ = ast_binary(x, @$, BINARY_BITAND, $1, $3); }
    | bits BITOR shift { $$ = ast_binary(x, @$, BINARY_BITOR, $1, $3); }
    ;

xor: bits { $$ = $1; }
    | xor XOR bits { $$ = ast_binary(x, @$, BINARY_XOR, $1, $3); }
    ;

and: xor { $$ = $1; }
    | and AND xor { $$ = ast_binary(x, @$, BINARY_AND, $1, $3); }
    ;

or: and { $$ = $1; }
    | or OR and { $$ = ast_binary(x, @$, BINARY_OR, $1, $3); }
    ;

expr: or { $$ = $1; }
    ;
    
%%
