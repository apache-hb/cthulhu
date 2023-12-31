%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch stack
%option prefix="cpp"

%{
#include "cpp_bison.h"
#include "base/log.h"
#include "cpp/scan.h"
#include "cpp/cpp.h"
#include "interop/flex.h"
#include "memory/arena.h"
%}

%x BLOCK_COMMENT
%x LINE_COMMENT
%x DIRECTIVE
%x INCLUDE
%x DEFINE

%%

"\""[^"\n]*"\"" {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text2.text = arena_strndup(yytext, yyleng, extra->instance->arena);
    yylval->text2.size = yyleng;
    return TOK_STRING;
}

\'[^\n]*\' {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text2.text = arena_strndup(yytext, yyleng, extra->instance->arena);
    yylval->text2.size = yyleng;
    return TOK_STRING;
}

<INITIAL,DIRECTIVE,INCLUDE,DEFINE>"//" {
    /*
    avoid matching inside a block comment
    because we leave via `BEGIN` rather than `yy_pop_state`
    */
    cpp_scan_consume(yyextra, yytext, yyleng, true);
    BEGIN(LINE_COMMENT);
}
<LINE_COMMENT>\n {
    cpp_scan_consume(yyextra, yytext, yyleng, true);
    /*
    we leave via BEGIN because line comments implicitly
    terminate directive statements
    */
    BEGIN(INITIAL);
}
<LINE_COMMENT>. {
    cpp_scan_consume(yyextra, yytext, yyleng, true);
}

<*>"/*" {
    /* block comments are handled here */
    cpp_scan_consume(yyextra, "/*", 2, true);
    yy_push_state(BLOCK_COMMENT, yyscanner);
}
<BLOCK_COMMENT>"*/" {
    cpp_scan_consume(yyextra, "*/", 2, true);
    yy_pop_state(yyscanner);
}
<BLOCK_COMMENT>.|\n {
    cpp_scan_consume(yyextra, yytext, yyleng, true);
}

<INITIAL>"#" {
    /* TODO: this accepts defines after the first token on the line */
    BEGIN(DIRECTIVE);
    return TOK_DIRECTIVE;
}

<LINE_COMMENT,DIRECTIVE,INCLUDE,DEFINE>\\\n { /* a backslash followed by a newline is a line continuation */ }
<LINE_COMMENT,DIRECTIVE,INCLUDE,DEFINE>\\\r\n { /* a backslash followed by a newline is a line continuation */ }

<DIRECTIVE,INCLUDE,DEFINE>[ \t\r\v\f]+ { /* consume all other (not newline) whitespace */ }

<DIRECTIVE,INCLUDE,DEFINE><<EOF>> {
    /* if we've reached the end of the file then the directive is complete */
    BEGIN(INITIAL);

    if (cpp_leave_file(yyscanner))
        yyterminate();
}

<DIRECTIVE,INCLUDE,DEFINE>\n {
    /* if we've reached an unescaped newline then the directive is complete */
    BEGIN(INITIAL);
}

<DIRECTIVE>"include" {
    BEGIN(INCLUDE);
    return TOK_INCLUDE;
}
<DIRECTIVE>"define" {
    BEGIN(DEFINE);
    return TOK_DEFINE;
}
<DIRECTIVE>"undef" {
    return TOK_UNDEF;
}
<DIRECTIVE>"if" { return TOK_IF; }
<DIRECTIVE>"ifdef" { return TOK_IFDEF; }
<DIRECTIVE>"ifndef" { return TOK_IFNDEF; }
<DIRECTIVE>"elif" { return TOK_ELIF; }
<DIRECTIVE>"else" { return TOK_ELSE; }
<DIRECTIVE>"endif" {
    /*
    TODO: need a way to return these as text when inside a define
    also need to avoid reporting these when inside a skipping group
    */
    return TOK_ENDIF;
}
<DIRECTIVE>"line" {
    return TOK_LINE;
}
<DIRECTIVE>"pragma" {
    return TOK_PRAGMA;
}
<DIRECTIVE>"error" {
    return TOK_ERROR;
}
<DIRECTIVE>"warning" {
    return TOK_WARNING;
}

<DIRECTIVE>"defined" { return TOK_PP_DEFINED; }

<DEFINE>[a-zA-Z_][a-zA-Z0-9_]*"(" {
    BEGIN(DIRECTIVE);

    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext, yyleng - 1, extra->instance->arena);
    return TOK_PP_NAME_MACRO;
}

<DIRECTIVE,DEFINE>[a-zA-Z_][a-zA-Z0-9_]* {
    BEGIN(DIRECTIVE);

    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext, yyleng, extra->instance->arena);
    return TOK_PP_IDENT;
}

<DIRECTIVE>"##" {
    return TOK_PP_CONCAT;
}

<DIRECTIVE>"#"[a-zA-Z0-9_]+ {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext + 1, yyleng - 1, extra->instance->arena);
    return TOK_PP_STRINGIFY;
}

<DIRECTIVE>"\""[^"\n]*"\"" {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext + 1, yyleng - 2, extra->instance->arena);
    return TOK_PP_STRING;
}

<INCLUDE>[a-zA-Z_][a-zA-Z0-9_]* {
    BEGIN(INITIAL);

    /*
    TODO: merge these in with <DIRECTIVE> rules
    this would require a touch of extra support in the parser and include logic
    but would be much cleaner. it would also allow us to include with string concatenation
    */

    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext, yyleng, extra->instance->arena);
    return TOK_PP_IDENT;
}

<INCLUDE>"\""[^"\n]*"\"" {
    BEGIN(INITIAL);

    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext + 1, yyleng - 2, extra->instance->arena);
    return TOK_PP_STRING;
}

<INCLUDE>"<"[^>\n]*">" {
    BEGIN(INITIAL);

    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext + 1, yyleng - 2, extra->instance->arena);
    return TOK_SYSTEM;
}

<DIRECTIVE>"(" { return TOK_PP_LPAREN; }
<DIRECTIVE>")" { return TOK_PP_RPAREN; }
<DIRECTIVE>"," { return TOK_PP_COMMA; }
<DIRECTIVE>"..." { return TOK_PP_ELLIPSIS; }
<DIRECTIVE>"!" { return TOK_PP_NOT; }
<DIRECTIVE>"&&" { return TOK_PP_AND; }
<DIRECTIVE>"||" { return TOK_PP_OR; }
<DIRECTIVE>"==" { return TOK_PP_EQ; }
<DIRECTIVE>"!=" { return TOK_PP_NE; }
<DIRECTIVE>"<=" { return TOK_PP_LE; }
<DIRECTIVE>">=" { return TOK_PP_GE; }
<DIRECTIVE>"<" { return TOK_PP_LT; }
<DIRECTIVE>">" { return TOK_PP_GT; }
<DIRECTIVE>"+" { return TOK_PP_PLUS; }
<DIRECTIVE>"-" { return TOK_PP_MINUS; }
<DIRECTIVE>"*" { return TOK_PP_MUL; }
<DIRECTIVE>"/" { return TOK_PP_DIV; }
<DIRECTIVE>"%" { return TOK_PP_MOD; }
<DIRECTIVE>"^" { return TOK_PP_XOR; }
<DIRECTIVE>"&" { return TOK_PP_BITAND; }
<DIRECTIVE>"|" { return TOK_PP_BITOR; }
<DIRECTIVE>"~" { return TOK_PP_BITNOT; }
<DIRECTIVE>"<<" { return TOK_PP_SHL; }
<DIRECTIVE>">>" { return TOK_PP_SHR; }
<DIRECTIVE>"?"  { return TOK_PP_QUESTION; }
<DIRECTIVE>":"  { return TOK_PP_COLON; }

<DEFINE>. {
    BEGIN(DIRECTIVE);
}

<DIRECTIVE>[0-9]+ {
    yylval->number = make_number(yyextra, yytext, yyleng, 10);
    return TOK_NUMBER;
}
<DIRECTIVE>"0x"[0-9a-fA-F]+ {
    yylval->number = make_number(yyextra, yytext + 2, yyleng - 2, 16);
    return TOK_NUMBER;
}
<DIRECTIVE>"0b"[01]+ {
    yylval->number = make_number(yyextra, yytext + 2, yyleng - 2, 2);
    return TOK_NUMBER;
}

<DIRECTIVE,INCLUDE>. {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext, yyleng, extra->instance->arena);
    return TOK_PP_PASTE;
}

"(" { return TOK_LPAREN; }
")" { return TOK_RPAREN; }
"," { return TOK_COMMA; }

<INITIAL>[a-zA-Z_][a-zA-Z0-9_]* {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text2.text = arena_strndup(yytext, yyleng, extra->instance->arena);
    yylval->text2.size = yyleng;
    return TOK_TEXT;
}

<INITIAL>[ \t\r\v\f]+ {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text2.text = arena_strndup(yytext, yyleng, extra->instance->arena);
    yylval->text2.size = yyleng;
    return TOK_WHITESPACE;
}

<INITIAL>.|\n {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text2.text = arena_strndup(yytext, yyleng, extra->instance->arena);
    yylval->text2.size = yyleng;
    return TOK_TEXT;
}

<<EOF>> {
    if (cpp_leave_file(yyscanner))
        yyterminate();
}

%%

FLEX_MEMORY(cppalloc, cpprealloc, cppfree)
