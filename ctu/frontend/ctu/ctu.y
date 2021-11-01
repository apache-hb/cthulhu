%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ctu}

%code requires {
    #define YYSTYPE CTUSTYPE
    #define YYLTYPE CTULTYPE
    
    #include "scan.h"
    #include "ast.h"
}

%{
#include "scan.h"
int ctulex();
void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    ctu_t *ctu;
    vector_t *vector;

    bool boolean;
    char *ident;
    mpz_t digit;
}

%token<ident>
    IDENT "identifier"
    STRING "string literal"

%token<digit>
    DIGIT "integer literal"

%token
    VAR "`var`"
    FINAL "`final`"
    DEF "`def`"
    EXPORT "`export`"
    RETURN "`return`"
    IF "`if`"
    ELSE "`else`"
    WHILE "`while`"
    AS "`as`"
    IMPORT "`import`"
    YES "`true`"
    NO "`false`"
    BREAK "`break`"
    LAMBDA "`lambda`"
    TYPE "`type`"
    NIL "`null`"
    SEMI "`;`"
    ASSIGN "`=`"
    LPAREN "`(`"
    RPAREN "`)`"
    LBRACE "`{`"
    RBRACE "`}`"
    LSQUARE "`[`"
    RSQUARE "`]`"
    COLON2 "`::`"
    COLON "`:`"
    COMMA "`,`"
    ELLIPSIS "`...`"
    DOT "`.`"
    ARROW "`->`"
    WIDEARROW "`=>`"
    ADD "`+`"
    SUB "`-`"
    MUL "`*`"
    DIV "`/`"
    REM "`%`"
    EQ "`==`"
    NEQ "`!=`"
    GT "`>`"
    GTE "`>=`"
    LT "`<`"
    LTE "`<=`"
    SHL "`<<`"
    SHR "`>>`"
    BITAND "`&`"
    BITOR "`|`"
    XOR "`^`"
    AND "`&&`"
    OR "`||`"
    TILDE "`~`"
    AT "`@`"
    END 0 

%type<ctu>
    value decl declbase
    expr primary postfix unary multiply add compare equality shift bits xor and or
    statements stmt type param function return import while
    assign attrib branch tail closure result funcbody lambda lambdabody newtype list

%type<vector>
    unit stmtlist params paramlist args arglist imports
    attributes attriblist attribute attribs
    typelist types optparams importlist path
    items itemlist

%type<boolean>
    exported const

%type<ident>
    alias

%start program

%%

program: importlist unit END 
    { scan_export(x, ctu_module(x, @$, $1, $2)); }
    ;

importlist: %empty { $$ = vector_new(0); }
    | imports { $$ = $1; }
    ;

imports: import { $$ = vector_init($1); }
    | imports import { vector_push(&$1, $2); $$ = $1; }
    ;

import: IMPORT path alias SEMI { $$ = ctu_import(x, @$, $2, $3); }
    ;

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

alias: %empty { $$ = NULL; }
    | AS IDENT { $$ = $2; }
    ;

unit: decl { $$ = vector_init($1); }
    | unit decl { vector_push(&$1, $2); $$ = $1; }
    ;

decl: attributes exported declbase { $$ = set_details($3, $1, $2); }
    ;

attributes: %empty { $$ = vector_new(0); }
    | attriblist { $$ = $1; }
    ;

attriblist: attribute { $$ = $1; }
    | attriblist attribute { $$ = vector_join($1, $2); }
    ;

attribute: AT attrib { $$ = vector_init($2); }
    | AT LSQUARE attribs RSQUARE { $$ = $3; }
    ;

attribs: attrib { $$ = vector_init($1); }
    | attribs COMMA attrib { vector_push(&$1, $3); $$ = $1; }
    ;

attrib: IDENT { $$ = ctu_attrib(x, @$, $1, vector_new(0)); }
    | IDENT LPAREN args RPAREN { $$ = ctu_attrib(x, @$, $1, $3); }
    ;

declbase: value { $$ = $1; }
    | function { $$ = $1; }
    | newtype { $$ = $1; }
    ;

newtype: TYPE IDENT ASSIGN type SEMI { $$ = ctu_newtype(x, @$, $2, $4); }
    ;

exported: %empty { $$ = false; }
    | EXPORT { $$ = true; }
    ;

value: const IDENT ASSIGN expr SEMI { $$ = ctu_value(x, @$, $1, $2, NULL, $4); }
    | const IDENT COLON type ASSIGN expr SEMI { $$ = ctu_value(x, @$, $1, $2, $4, $6); }
    | const IDENT COLON type SEMI { $$ = ctu_value(x, @$, $1, $2, $4, NULL); }
    ;

const: VAR { $$ = true; }
    | FINAL { $$ = false; }
    ;

function: DEF IDENT optparams result funcbody 
    { $$ = ctu_define(x, @$, $2, $3, $4, $5); }
    | DEF IDENT optparams result SEMI 
    { $$ = ctu_define(x, @$, $2, $3, $4, NULL); }
    ;

funcbody: statements { $$ = $1; }
    | ASSIGN expr SEMI { $$ = ctu_stmts(x, @$, vector_init(ctu_return(x, @$, $2))); }
    ;

lambdabody: statements { $$ = $1; }
    | WIDEARROW expr { $$ = ctu_stmts(x, @$, vector_init(ctu_return(x, @$, $2))); }
    ;

result: %empty { $$ = ctu_typename(x, @$, "void"); }
    | COLON type { $$ = $2; }
    ;

optparams: %empty { $$ = vector_new(0); }
    | LPAREN paramlist RPAREN { $$ = $2; }
    ;

paramlist: %empty { $$ = vector_new(0); }
    | params { $$ = $1; }
    ;

params: param { $$ = vector_init($1); }
    | params COMMA param { vector_push(&$1, $3); $$ = $1; }
    ;

param: IDENT COLON type { $$ = ctu_param(x, @$, $1, $3); }
    | IDENT COLON ELLIPSIS { $$ = ctu_param(x, @$, $1, ctu_varargs(x, @3)); }
    ;

type: path { $$ = ctu_typepath(x, @$, $1); }
    | MUL type { $$ = ctu_pointer(x, @$, $2, false); }
    | LSQUARE type RSQUARE { $$ = ctu_pointer(x, @$, $2, true); }
    | LSQUARE type MUL expr RSQUARE { $$ = ctu_array(x, @$, $2, $4); }
    | closure { $$ = $1; }
    | VAR type { $$ = ctu_mutable(x, @$, $2); }
    ;

closure: LPAREN typelist RPAREN ARROW type { $$ = ctu_closure(x, @$, $2, $5); }
    ;

typelist: %empty { $$ = vector_new(0); }
    | types { $$ = $1; }
    ;

types: type { $$ = vector_init($1); }
    | types COMMA type { vector_push(&$1, $3); $$ = $1; }
    ;

statements: LBRACE RBRACE { $$ = ctu_stmts(x, @$, vector_new(0)); } 
    | LBRACE stmtlist RBRACE { $$ = ctu_stmts(x, @$, $2); }
    ;

stmtlist: stmt { $$ = vector_init($1); }
    | stmtlist stmt { vector_push(&$1, $2); $$ = $1; }
    ;

stmt: expr SEMI { $$ = $1; }
    | value { $$ = $1; }
    | statements { $$ = $1; }
    | return { $$ = $1; }
    | while { $$ = $1; }
    | assign { $$ = $1; }
    | branch { $$ = $1; }
    | BREAK SEMI { $$ = ctu_break(x, @$); }
    ;

branch: IF expr statements tail { $$ = ctu_branch(x, @$, $2, $3, $4); }
    ;

tail: %empty { $$ = NULL; }
    | ELSE statements { $$ = $2; }
    | ELSE branch { $$ = $2; }
    ;

assign: expr ASSIGN expr SEMI { $$ = ctu_assign(x, @$, $1, $3); }
    ;

while: WHILE expr statements { $$ = ctu_while(x, @$, $2, $3); }
    ;

return: RETURN SEMI { $$ = ctu_return(x, @$, NULL); }
    | RETURN expr SEMI { $$ = ctu_return(x, @$, $2); }
    ;

arglist: %empty { $$ = vector_new(0); }
    | args { $$ = $1; }
    ;

args: expr { $$ = vector_init($1); }
    | args COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

lambda: LAMBDA optparams result lambdabody { $$ = ctu_lambda(x, @$, $2, $3, $4); }
    ;

itemlist: expr { $$ = vector_init($1); }
    | itemlist COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

items: %empty { $$ = vector_new(0); }
    | itemlist { $$ = $1; }
    ;

list: LSQUARE items RSQUARE { $$ = ctu_list(x, @$, $2); }
    ;

primary: LPAREN expr RPAREN { $$ = $2; }
    | path { $$ = ctu_path(x, @$, $1); }
    | DIGIT { $$ = ctu_digit(x, @$, $1); }
    | YES { $$ = ctu_bool(x, @$, true); }
    | NO { $$ = ctu_bool(x, @$, false); }
    | STRING { $$ = ctu_string(x, @$, $1); }
    | NIL { $$ = ctu_null(x, @$); }
    | list { $$ = $1; }
    ;

postfix: primary { $$ = $1; }
    | postfix LPAREN arglist RPAREN { $$ = ctu_call(x, @$, $1, $3); }
    //| postfix DOT IDENT { $$ = ctu_access(x, @$, $1, $3, false); }
    //| postfix ARROW IDENT { $$ = ctu_access(x, @$, $1, $3, true); }
    | postfix AS type { $$ = ctu_cast(x, @$, $1, $3); }
    | postfix LSQUARE expr RSQUARE { $$ = ctu_index(x, @$, $1, $3); }
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = ctu_unary(x, @$, UNARY_ABS, $2); }
    | SUB unary { $$ = ctu_unary(x, @$, UNARY_NEG, $2); }
    | BITAND unary { $$ = ctu_unary(x, @$, UNARY_ADDR, $2); }
    | MUL unary { $$ = ctu_unary(x, @$, UNARY_DEREF, $2); }
    | TILDE unary { $$ = ctu_unary(x, @$, UNARY_BITFLIP, $2); }
    ;

multiply: unary { $$ = $1; }
    | multiply MUL unary { $$ = ctu_binary(x, @$, BINARY_MUL, $1, $3); }
    | multiply DIV unary { $$ = ctu_binary(x, @$, BINARY_DIV, $1, $3); }
    | multiply REM unary { $$ = ctu_binary(x, @$, BINARY_REM, $1, $3); }
    ;

add: multiply { $$ = $1; }
    | add ADD multiply { $$ = ctu_binary(x, @$, BINARY_ADD, $1, $3); }
    | add SUB multiply { $$ = ctu_binary(x, @$, BINARY_SUB, $1, $3); }
    ;

compare: add { $$ = $1; }
    | compare GT add { $$ = ctu_binary(x, @$, BINARY_GT, $1, $3); }
    | compare GTE add { $$ = ctu_binary(x, @$, BINARY_GTE, $1, $3); }
    | compare LT add { $$ = ctu_binary(x, @$, BINARY_LT, $1, $3); }
    | compare LTE add { $$ = ctu_binary(x, @$, BINARY_LTE, $1, $3); }
    ;

equality: compare { $$ = $1; }
    | equality EQ compare { $$ = ctu_binary(x, @$, BINARY_EQ, $1, $3); }
    | equality NEQ compare { $$ = ctu_binary(x, @$, BINARY_NEQ, $1, $3); }
    ;

shift: equality { $$ = $1; }
    | shift SHL equality { $$ = ctu_binary(x, @$, BINARY_SHL, $1, $3); }
    | shift SHR equality { $$ = ctu_binary(x, @$, BINARY_SHR, $1, $3); }
    ;

bits: shift { $$ = $1; }
    | bits BITAND shift { $$ = ctu_binary(x, @$, BINARY_BITAND, $1, $3); }
    | bits BITOR shift { $$ = ctu_binary(x, @$, BINARY_BITOR, $1, $3); }
    ;

xor: bits { $$ = $1; }
    | xor XOR bits { $$ = ctu_binary(x, @$, BINARY_XOR, $1, $3); }
    ;

and: xor { $$ = $1; }
    | and AND xor { $$ = ctu_binary(x, @$, BINARY_AND, $1, $3); }
    ;

or: and { $$ = $1; }
    | or OR and { $$ = ctu_binary(x, @$, BINARY_OR, $1, $3); }
    ;

expr: or { $$ = $1; }
    | lambda { $$ = $1; }
    ;
    
%%
