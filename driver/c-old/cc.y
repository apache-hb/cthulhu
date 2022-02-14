%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cc}

%code requires {
    #define YYSTYPE CCSTYPE
    #define YYLTYPE CCLTYPE
    
    #include "scan.h"
    #include "sema.h"
}

%{
#include "scan.h"
int cclex();
void ccerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    ast_t *ast;

    char *ident;
    vector_t *vector;

    type_t *type;

    digit_t digit;
    sign_t sign;

    vardecl_t *vardecl;
    param_t *param;

    param_pack_t *param_pack;

    mpz_t mpz;
}

%token<ident>
    IDENT "identifier"
    TYPENAME "typename"
    STRING "string"

%token<mpz>
    DIGIT "integer literal"

%token
    /* module extensions */
    MODULE "_Module"
    IMPORT "_Import"

    EXTERN "extern"
    STATIC "static"
    AUTO "auto"

    SIGNED "signed"
    UNSIGNED "unsigned"

    CHAR "char"
    SHORT "short"
    INT "int"
    LONG "long"

    BOOL "_Bool"
    VOID "void"

    CONST "const"
    VOLATILE "volatile"

    SEMI ";"
    COMMA ","
    COLON2 "::"
    ASSIGN "="
    QUESTION "?"
    COLON ":"

    ADD "+"
    SUB "-"
    STAR "*"
    DIV "/"
    REM "%"

    DOT3 "..."
    DOT "."

    LBRACE "{"
    RBRACE "}"

    LPAREN "("
    RPAREN ")"

%type<vector>
    path varlist
    params funcbody
    opt_args args

%type<vardecl>
    var

%type<type>
    type elaborate_type opt_type full_type integral

%type<sign>
    sign

%type<digit>
    inttype

%type<ast>
    expr primary postfix unary mul add

%type<param>
    param

%type<ident>
    opt_name

%type<param_pack>
    opt_params_with_varargs

%start program

%%

program: modspec opt_imports { cc_finish(x, @$); }
       | modspec opt_imports decls { cc_finish(x, @$); }
       ;

decls: decl | decls decl ;

decl: storage opt_type declbody 
    ;

declbody: varlist SEMI { cc_vardecl(x, $1); }
        | elaborate_type IDENT LPAREN opt_params_with_varargs RPAREN funcbody { cc_funcdecl(x, node_new(x, @$), $2, $4, $6); }
        ;

opt_params_with_varargs: params { $$ = new_param_pack($1, false); }
                       | params COMMA DOT3 { $$ = new_param_pack($1, true); }
                       | %empty { $$ = new_param_pack(vector_new(0), true); }
                       ;

params: param { $$ = vector_init($1); }
      | params COMMA param { vector_push(&$1, $3); $$ = $1; }
      ;

opt_name: %empty { $$ = NULL; }
        | IDENT { $$ = $1; }
        ;

param: full_type opt_name { $$ = new_param(x, @$, $1, $2); }
     ;

funcbody: SEMI { $$ = NULL; }
        | LBRACE stmts RBRACE { $$ = vector_new(0); }
        ;

storage: %empty { set_storage(x, LINK_EXPORTED); }
       | EXTERN { set_storage(x, LINK_IMPORTED); }
       | STATIC { set_storage(x, LINK_INTERNAL); }
       | AUTO { set_storage(x, LINK_EXPORTED); }
       ;

varlist: var { $$ = vector_init($1); }
       | varlist COMMA var { vector_push(&$1, $3); $$ = $1; }
       ;

var: elaborate_type IDENT { $$ = new_vardecl(x, @$, $1, $2, NULL); }
   | elaborate_type IDENT ASSIGN expr { $$ = new_vardecl(x, @$, $1, $2, $4); }
   ;

full_type: type elaborate_type { $$ = $2; }
         ;

opt_type: %empty { $$ = default_int(x, node_new(x, @$)); }
        | type { $$ = $1; }
        ;

elaborate_type: %empty { $$ = get_current_type(x); }
              | STAR elaborate_type { $$ = type_pointer("pointer", get_current_type(x), node_new(x, @$)); }
              ;

type: opt_mods integral { $$ = $2; }
    | integral { $$ = $1; }
    ;

opt_mods: typemod | opt_mods typemod ;
typemod: CONST | VOLATILE ;

integral: sign { $$ = get_digit($1, DIGIT_INT); set_current_type(x, $$); }
    | inttype { $$ = get_digit(SIGN_DEFAULT, $1); set_current_type(x, $$); }
    | sign inttype { $$ = get_digit($1, $2); set_current_type(x, $$); }
    | VOID { $$ = get_void(); set_current_type(x, $$); }
    | BOOL { $$ = get_bool(); set_current_type(x, $$); }
    ;

inttype: CHAR { $$ = DIGIT_CHAR; }
       | SHORT { $$ = DIGIT_SHORT; }
       | INT { $$ = DIGIT_INT; }
       | LONG { $$ = DIGIT_LONG; }
       | SHORT INT { $$ = DIGIT_SHORT; }
       | LONG INT { $$ = DIGIT_LONG; }
       | LONG LONG { $$ = DIGIT_LONG; }
       | LONG LONG INT { $$ = DIGIT_LONG; }
       ;

sign: SIGNED { $$ = SIGN_SIGNED; }
    | UNSIGNED { $$ = SIGN_UNSIGNED; }
    ;

primary: DIGIT { $$ = ast_digit(x, @$, $1); } 
       | IDENT { $$ = ast_ident(x, @$, $1); }
       | STRING { $$ = ast_string(x, @$, $1); }
       ;

postfix: primary { $$ = $1; }
       | postfix LPAREN opt_args RPAREN { $$ = $1; }
       ;

opt_args: %empty { $$ = vector_new(0); }
    | args { $$ = $1; }
    ;

args: expr { $$ = vector_init($1); }
    | args COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

unary: postfix { $$ = $1; }
     | ADD unary { $$ = ast_unary(x, @$, $2, UNARY_ABS); }
     | SUB unary { $$ = ast_unary(x, @$, $2, UNARY_NEG); }
     ;

mul: unary { $$ = $1; }
    | mul STAR unary { $$ = ast_binary(x, @$, $1, $3, BINARY_MUL); }
    | mul DIV unary { $$ = ast_binary(x, @$, $1, $3, BINARY_DIV); }
    | mul REM unary { $$ = ast_binary(x, @$, $1, $3, BINARY_REM); }
    ;

add: mul { $$ = $1; }
    | add ADD mul { $$ = ast_binary(x, @$, $1, $3, BINARY_ADD); }
    | add SUB mul { $$ = ast_binary(x, @$, $1, $3, BINARY_SUB); }
    ;

expr: add { $$ = $1; }
    ;

stmt: expr SEMI
    | LBRACE stmts RBRACE
    ;

stmts: stmt
     | stmts stmt
     ;

modspec: %empty
       | MODULE path SEMI { cc_module(x, $2); }
       ;

opt_imports: %empty | imports ;
imports: import | imports import ;
import: IMPORT path SEMI ;

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

%%
