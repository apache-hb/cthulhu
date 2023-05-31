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
    #include "ast.h"
    #include "scan.h"
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

    mpz_t digit;
}

%token<ident>
    IDENT "identifier"

%token<digit>
    DIGIT "digit"

%token<string>
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

    TOK_NORETURN "_Noreturn"

    STATIC_ASSERT "_Static_assert"
    THREAD_LOCAL "_Thread_local"

    STRUCT "struct"
    UNION "union"
    ENUM "enum"

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
