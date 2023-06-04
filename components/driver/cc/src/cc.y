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

    mpz_t mpz;

    vector_t *vector;

    cc_t *ast;
    
    sign_t sign;
    digit_t digit;
}

%token<ident>
    IDENT "identifier"

%token<mpz>
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

%type<vector>
    path modspec
    imports importlist
    decls

%type<ast>
    import
    decl typedefDecl type

%type<sign>
    sign

%type<digit>
    digit

%start unit

%%

unit: modspec imports decls { scan_set(x, cc_module(x, @$, $1, $2, $3)); }
    ;

imports: %empty { $$ = vector_new(0); }
    | importlist { $$ = $1; }
    ;

importlist: import { $$ = vector_init($1); }
    | importlist import { vector_push(&$1, $2); $$ = $1; }
    ;

import: IMPORT path SEMICOLON { $$ = cc_import(x, @$, $2, NULL); }
    | IMPORT path COLON IDENT SEMICOLON { $$ = cc_import(x, @$, $2, $4); }
    ;

modspec: %empty { $$ = NULL; }
    | MODULE path SEMICOLON { $$ = $2; }
    ;

decls: decl { $$ = vector_init($1); }
    | decls decl { vector_push(&$1, $2); $$ = $1; }
    ;

decl: typedefDecl { $$ = $1; }
    ;

typedefDecl: TYPEDEF IDENT type SEMICOLON { $$ = cc_typedef(x, @$, $2, $3); }
    ;

type: sign digit { $$ = cc_digit(x, @$, $1, $2); }
    | BOOL { $$ = cc_bool(x, @$); }
    | type MUL { $$ = cc_pointer(x, @$, $1); }
    ;

sign: %empty { $$ = eSigned; }
    | SIGNED { $$ = eSigned; }
    | UNSIGNED { $$ = eUnsigned; }
    ;

digit: CHAR { $$ = eDigitChar; }
    | SHORT { $$ = eDigitShort; }
    | INT { $$ = eDigitInt; }
    | LONG { $$ = eDigitLong; }
    ;

path: IDENT { $$ = vector_init($1); }
    | path DOT IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

%%
