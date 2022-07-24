%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cc}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "scan.h"
    #include "ast.h"
    
    #define YYSTYPE CCSTYPE
    #define YYLTYPE CCLTYPE
}

%{
int cclex();
void ccerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;

    struct {
        char *text;
        size_t length;
    } string;
}

%token<ident>
    IDENT "identifier"

%token
    TYPEDEF "typedef"

    VOID "void"
    BOOL "_Bool"
    
    CHAR "char"
    SHORT "short"
    INT "int"
    LONG "long"

    FLOAT "float"
    DOUBLE "double"

    SIGNED "signed"
    UNSIGNED "unsigned"

    ATOMIC "_Atomic"
    CONST "const"
    VOLATILE "volatile"

    STRUCT "struct"
    UNION "union"

    SEMICOLON ";"

%start unit

%%

unit: decls
    ;

decls: decl
    | decls decl
    ;

decl: typedefDecl
    ;

typedefDecl: TYPEDEF type IDENT SEMICOLON
    ;

type: sign integral
    ;

sign: %empty 
    | SIGNED
    | UNSIGNED
    ;

integral: BOOL
    | CHAR
    | SHORT
    | INT
    | LONG
    | LONG LONG
    | FLOAT
    | DOUBLE
    ;

%%
