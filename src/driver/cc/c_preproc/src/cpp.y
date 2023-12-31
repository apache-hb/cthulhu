%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%expect 0
%define api.prefix {cpp}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "base/log.h"
    #include "cpp/scan.h"
    #define YYSTYPE CPPSTYPE
}

%{
int cpplex(void *yylval, void *yyscanner);
void cpperror(void *state, scan_t *scan, const char *msg);
%}

%union {
    char *text;
}

%token<text>
    TOK_IDENT "identifier"
    // double quote string literals
    TOK_STRING "string literal"
    // <> include file
    TOK_SYSTEM "system header"

%token
    TOK_DIRECTIVE "#"
    TOK_INCLUDE "include"
    TOK_DEFINE "define"
    TOK_UNDEF "undef"
    TOK_PRAGMA "pragma"
    TOK_ERROR "error"

%start entry

%%

entry: %empty
    | entry any
    ;

any: TOK_DIRECTIVE directive
    | TOK_IDENT
    | error
    ;

directive: directive_include
    | directive_define
    | directive_undef
    | directive_pragma
    | directive_error
    | TOK_IDENT { }
    ;

directive_include: TOK_INCLUDE TOK_STRING { cpp_accept_include(scan, $2); }
    | TOK_INCLUDE TOK_SYSTEM { cpp_accept_include(scan, $2); }
    | TOK_INCLUDE TOK_IDENT { cpp_accept_define_include(scan, $2); }
    ;

directive_define: TOK_DEFINE TOK_IDENT { }
    | TOK_DEFINE TOK_IDENT TOK_STRING { }
    ;

directive_undef: TOK_UNDEF TOK_IDENT { }
    ;

directive_pragma: TOK_PRAGMA TOK_IDENT { }
    ;

directive_error: TOK_ERROR TOK_STRING { }
    ;

%%
