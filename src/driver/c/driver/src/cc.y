%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cc}

%code top {
    #include "interop/flex.h"
    #include "cthulhu/tree/ops.h"
}

%code requires {
    #include "cc/ast.h"
    #include "cc/scan.h"
    #define YYSTYPE CCSTYPE
    #define YYLTYPE CCLTYPE
}

%{
int cclex(void *yylval, void *yylloc, void *yyscanner);
void ccerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;

    text_t text;

    mpz_t mpz;

    vector_t *vector;

    sign_t sign;
    digit_t digit;
}

%token<ident>
    IDENT "identifier"

%token<mpz>
    DIGIT "digit"

%token<text>
    STRING "string"

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
    RESTRICT "restrict"
    AUTO "auto"
    EXTERN "extern"
    INLINE "inline"
    STATIC "static"
    REGISTER "register"

    BREAK "break"
    CASE "case"
    CONTINUE "continue"
    DEFAULT "default"

    DO "do"
    ELSE "else"
    WHILE "while"
    FOR "for"
    GOTO "goto"
    IF "if"
    RETURN "return"
    SWITCH "switch"

    SIZEOF "sizeof"
    ALIGNAS "_Alignas"
    ALIGNOF "_Alignof"

    COMPLEX "_Complex"
    GENERIC "_Generic"
    IMAGINARY "_Imaginary"

    NRETURN "_Noreturn"

    STATIC_ASSERT "_Static_assert"
    THREAD_LOCAL "_Thread_local"

    STRUCT "struct"
    UNION "union"
    ENUM "enum"

    SEMICOLON ";"

    COLON2 "::"
    COLON ":"

    DOT "."

    MULEQ "*="
    MUL "*"

    MODULE "_Module"
    IMPORT "_Import"
    EXPORT "_Export"
    PRIVATE "_Private"

%start unit

%%

unit: module_decl SEMICOLON decl_list
    | decl_list
    ;

decl_list: decl
    | decl_list decl
    ;

module_decl: MODULE
    | MODULE module_path module_partition
    | EXPORT MODULE module_path module_partition
    | MODULE COLON PRIVATE
    ;

module_partition: %empty
    | COLON module_path
    ;

module_path: IDENT
    | module_path DOT IDENT
    ;

opt_export: %empty
    | EXPORT
    ;

decl: opt_export simple_decl SEMICOLON
    | error
    ;

simple_decl: STRUCT IDENT
    | UNION IDENT
    | ENUM IDENT
    | IMPORT module_path module_partition
    ;

%%
