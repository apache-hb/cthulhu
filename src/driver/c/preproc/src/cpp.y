%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { cpp_extra_t *x }
%locations
%expect 0
%define api.prefix {cpp}

%code top {
    #include "cpp/scan.h"
}

%code requires {
    #include "core/text.h"
    #include "cpp/scan.h"
    #include "base/log.h"
    #define YYSTYPE CPPSTYPE
    #define YYLTYPE CPPLTYPE
}

%{
int cpplex(void *yylval, void *yylloc, void *yyscanner);
void cpperror(where_t *where, void *state, cpp_extra_t *extra, const char *msg);
%}

%union {
    text_t text;
    char paste;
}

%token<text>
    TOK_WHITESPACE "whitespace"
    TOK_BLOCK_COMMENT "block comment"
    TOK_IDENT "identifier"
    TOK_STRING "string literal"
    TOK_LINE_COMMENT

%token<paste> TOK_PASTE

%token
    TOK_BEGIN_DIRECTIVE "#"
    TOK_END_INCLUDE
    TOK_END_DIRECTIVE

    /* preprocessor directives */
    TOK_INCLUDE "include"
    TOK_DEFINE "define"
    TOK_UNDEF "undef"
    TOK_IFDEF "ifdef"
    TOK_IFNDEF "ifndef"
    TOK_IF "if"
    TOK_ELIF "elif"
    TOK_ELSE "else"
    TOK_ENDIF "endif"
    TOK_ERROR "error"
    TOK_WARNING "warning"
    TOK_PRAGMA "pragma"
    TOK_LINE "line"

%start entry

%%

entry: %empty
    | entry item
    ;

item: TOK_IDENT { cpp_push_output(x, $1); }
    | TOK_STRING { cpp_push_output(x, $1); }
    | TOK_PASTE { cpp_push_output_single(x, $1); }
    | TOK_BEGIN_DIRECTIVE ws directive end_directive
    | TOK_BEGIN_DIRECTIVE ws TOK_INCLUDE ws end_include
    | TOK_BLOCK_COMMENT { cpp_push_output(x, $1); }
    | TOK_WHITESPACE { cpp_push_output(x, $1); }
    | TOK_LINE_COMMENT { cpp_push_output(x, $1); }
    ;

end_directive: TOK_END_DIRECTIVE
    | TOK_LINE_COMMENT { cpp_push_output(x, $1); }
    ;

end_include: TOK_END_INCLUDE
    | TOK_LINE_COMMENT { cpp_push_output(x, $1); }
    ;

directive: TOK_IDENT directive_body
    | TOK_DEFINE ws TOK_IDENT directive_body { cpp_add_define(x, @$, $3); }
    | TOK_UNDEF ws TOK_IDENT ws { cpp_remove_define(x, @$, $3); }
    | TOK_IFDEF ws TOK_IDENT ws { cpp_ifdef(x, @$, $3); }
    | TOK_IFNDEF ws TOK_IDENT ws { cpp_ifndef(x, @$, $3); }
    | TOK_IF directive_body { cpp_if(x, @$); }
    | TOK_ELIF directive_body { cpp_elif(x, @$); }
    | TOK_ELSE ws { cpp_else(x, @$); }
    | TOK_ENDIF ws { cpp_endif(x, @$); }
    | TOK_ERROR directive_body
    | TOK_WARNING directive_body
    | TOK_PRAGMA directive_body
    | TOK_LINE directive_body
    ;

directive_body: %empty
    | directive_body directive_item
    ;

directive_item: TOK_IDENT
    | TOK_WHITESPACE
    | TOK_BLOCK_COMMENT
    | TOK_STRING
    | TOK_PASTE
    ;

ws: %empty
    | ws TOK_WHITESPACE
    | ws TOK_BLOCK_COMMENT
    ;

%%