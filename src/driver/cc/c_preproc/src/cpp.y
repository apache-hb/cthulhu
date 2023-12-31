%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cpp}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "cpp/scan.h"
    #define YYSTYPE CPPSTYPE
    #define YYLTYPE CPPLTYPE
}

%{
int cpplex(void *yylval, void *yylloc, void *yyscanner);
void cpperror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *text;
}

%token<text>
    // produced when outside a directive
    TOK_IDENT "identifier"
    // produced when inside a directive
    TOK_PP_IDENT "identifier (preprocessor)"
    // produced when a macro has arguments
    TOK_PP_NAME_MACRO "name of a macro (preprocessor)"
    // double quote string literals
    TOK_STRING "string literal"
    // double quote string literal appearing in a directive
    TOK_PP_STRING "string literal (preprocessor)"
    // #id inside a define directive
    TOK_PP_STRINGIFY "macro stringification (preprocessor)"

    TOK_PP_PASTE "token paste"
    // <> include file
    TOK_SYSTEM "system header"

%token
    TOK_DIRECTIVE "#"
    TOK_INCLUDE "include"
    TOK_DEFINE "define"
    TOK_UNDEF "undef"
    TOK_IF "if"
    TOK_IFDEF "ifdef"
    TOK_IFNDEF "ifndef"
    TOK_ELSE "else"
    TOK_ELIF "elif"
    TOK_ENDIF "endif"
    TOK_LINE "line"
    TOK_PRAGMA "pragma"
    TOK_ERROR "error"
    TOK_WARNING "warning"

    TOK_LPAREN "`(`"
    TOK_RPAREN "`)`"
    TOK_COMMA "`,`"

    TOK_PP_LPAREN "`(` (directive)"
    TOK_PP_RPAREN "`)` (directive)"
    TOK_PP_COMMA "`,` (directive)"
    TOK_PP_ELLIPSIS "`...` (directive)"
    TOK_PP_CONCAT "`##` (concat)"

%start entry

%%

entry: %empty
    | entry any
    ;

any: TOK_DIRECTIVE directive
    | TOK_IDENT
    | TOK_STRING
    | TOK_LPAREN
    | TOK_RPAREN
    | TOK_COMMA
    | error
    ;

directive: directive_include
    | directive_define
    | directive_undef
    | directive_if
    | directive_ifdef
    | directive_ifndef
    | directive_else
    | directive_elif
    | directive_endif
    | directive_line
    | directive_pragma
    | directive_error
    | directive_warning
    | TOK_PP_IDENT
    | TOK_PP_PASTE
    ;

directive_include: TOK_INCLUDE TOK_PP_STRING { cpp_accept_include(scan, $2); }
    | TOK_INCLUDE TOK_SYSTEM { cpp_accept_include(scan, $2); }
    | TOK_INCLUDE TOK_PP_IDENT { cpp_accept_define_include(scan, $2); }
    ;

directive_define: TOK_DEFINE TOK_PP_IDENT message_body
    | TOK_DEFINE TOK_PP_NAME_MACRO define_opt_params TOK_PP_RPAREN message_body
    ;

define_opt_params: %empty
    | define_params
    | define_params TOK_PP_COMMA TOK_PP_ELLIPSIS
    ;

define_params: define_param
    | define_params TOK_PP_COMMA define_param
    ;

define_param: TOK_PP_IDENT
    ;

directive_undef: TOK_UNDEF message_body
    ;

directive_if: TOK_IF message_body
    ;

directive_ifdef: TOK_IFDEF message_body
    ;

directive_ifndef: TOK_IFNDEF message_body
    ;

directive_else: TOK_ELSE
    ;

directive_elif: TOK_ELIF message_body
    ;

directive_endif: TOK_ENDIF
    ;

directive_line: TOK_LINE message_body
    ;

directive_pragma: TOK_PRAGMA message_body
    ;

directive_error: TOK_ERROR message_body
    ;

directive_warning: TOK_WARNING message_body
    ;

message_body: %empty
    | message_body message_item
    ;

message_item: TOK_WARNING
    | TOK_ERROR
    | TOK_PP_STRING
    | TOK_PP_IDENT
    | TOK_PP_LPAREN
    | TOK_PP_RPAREN
    | TOK_PP_COMMA
    | TOK_PP_ELLIPSIS
    | TOK_PP_STRINGIFY
    | TOK_PP_CONCAT
    | TOK_PP_PASTE
    | TOK_IF
    | TOK_ELSE
    ;

%%
