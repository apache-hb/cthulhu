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
    DEF "`def`"
    EXPORT "`export`"
    RETURN "`return`"
    IF "`if`"
    ELSE "`else`"
    WHILE "`while`"
    IMPORT "`import`"
    YES "`true`"
    NO "`false`"
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
    AT "`@`"
    END 0 

%type<ctu>
    value decl declbase
    expr primary postfix unary multiply add compare equality shift bits xor and or
    statements stmt type param function return import while
    assign attrib branch

%type<vector>
    unit stmtlist params paramlist args arglist imports
    attributes attriblist attribute attribs

%type<boolean>
    exported

%start program

%%

program: unit END { scan_export(x, ctu_module(x, @$, $1)); }
    | imports unit END { scan_export(x, ctu_module(x, @$, $2)); }
    ;

imports: import { $$ = vector_init($1); }
    | imports import { vector_push(&$1, $2); $$ = $1; }
    ;

import: IMPORT { $$ = NULL; }
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
    | attribs attrib { vector_push(&$1, $2); $$ = $1; }
    ;

attrib: IDENT { $$ = ctu_attrib(x, @$, $1, vector_new(0)); }
    | IDENT LPAREN args RPAREN { $$ = ctu_attrib(x, @$, $1, $3); }
    ;

declbase: value { $$ = $1; }
    | function { $$ = $1; }
    ;

exported: %empty { $$ = false; }
    | EXPORT { $$ = true; }
    ;

value: VAR IDENT ASSIGN expr SEMI { $$ = ctu_value(x, @$, $2, NULL, $4); }
    | VAR IDENT COLON type ASSIGN expr SEMI { $$ = ctu_value(x, @$, $2, $4, $6); }
    | VAR IDENT COLON type SEMI { $$ = ctu_value(x, @$, $2, $4, NULL); }
    ;

function: DEF IDENT LPAREN paramlist RPAREN COLON type statements 
    { $$ = ctu_define(x, @$, $2, $4, $7, $8); }
    | DEF IDENT LPAREN paramlist RPAREN COLON type ASSIGN expr SEMI
    { $$ = ctu_define(x, @$, $2, $4, $7, ctu_stmts(x, @9, vector_init(ctu_return(x, @9, $9)))); }
    | DEF IDENT LPAREN paramlist RPAREN COLON type SEMI 
    { $$ = ctu_define(x, @$, $2, $4, $7, NULL); }
    ;

paramlist: %empty { $$ = vector_new(0); }
    | params { $$ = $1; }
    ;

params: param { $$ = vector_init($1); }
    | params COMMA param { vector_push(&$1, $3); $$ = $1; }
    ;

param: IDENT COLON type { $$ = ctu_param(x, @$, $1, $3); }
    ;

type: IDENT { $$ = ctu_typename(x, @$, $1); }
    | MUL type { $$ = ctu_pointer(x, @$, $2); }
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
    ;

branch: IF expr statements { $$ = ctu_branch(x, @$, $2, $3); }
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

primary: LPAREN expr RPAREN { $$ = $2; }
    | IDENT { $$ = ctu_ident(x, @$, $1); }
    | DIGIT { $$ = ctu_digit(x, @$, $1); }
    | YES { $$ = ctu_bool(x, @$, true); }
    | NO { $$ = ctu_bool(x, @$, false); }
    | STRING { $$ = ctu_string(x, @$, $1); }
    ;

postfix: primary { $$ = $1; }
    | postfix LPAREN arglist RPAREN { $$ = ctu_call(x, @$, $1, $3); }
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = ctu_unary(x, @$, UNARY_ABS, $2); }
    | SUB unary { $$ = ctu_unary(x, @$, UNARY_NEG, $2); }
    | BITAND unary { $$ = ctu_unary(x, @$, UNARY_ADDR, $2); }
    | MUL unary { $$ = ctu_unary(x, @$, UNARY_DEREF, $2); }
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
    ;
    
%%