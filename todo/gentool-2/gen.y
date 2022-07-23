%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {gen}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "gen.h"
    
    #define YYSTYPE GENSTYPE
    #define YYLTYPE GENLTYPE
}

%{
int genlex();
void generror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *text;

    struct {
        char *data;
        size_t length;
    } string;
}

%token<text>
    IDENT "identifier"

%token<string>
    STRING "string literal"

%token
    LBRACE "{"
    RBRACE "}"

    LPAREN "("
    RPAREN ")"

    COLON ":"

%start program

%%

program: sections ;

sections: section
    | sections section
    ;

section: IDENT LBRACE fields RBRACE
    ;

fields: field
    | fields field
    ;

field: name body
    ;

name: IDENT
    | IDENT COLON IDENT
    ;

body: LBRACE fields RBRACE
    | STRING
    | LPAREN content RPAREN
    ;

content: STRING
    | IDENT
    | STRING STRING
    ;

%%
