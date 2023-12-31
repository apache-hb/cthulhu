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
    yylval->text = arena_strndup(yytext + 1, yyleng - 2, extra->instance->arena);
    return TOK_STRING;
}

\'[^\n]*\' {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext + 1, yyleng - 2, extra->instance->arena);
    return TOK_STRING;
}

<INITIAL,DIRECTIVE,INCLUDE,DEFINE>"//" {
    /*
    avoid matching inside a block comment
    because we leave via `BEGIN` rather than `yy_pop_state`
    */
    cpp_scan_consume(yyextra, yytext, yyleng);
    BEGIN(LINE_COMMENT);
}
<LINE_COMMENT>\n {
    cpp_scan_consume(yyextra, yytext, yyleng);
    /*
    we leave via BEGIN because line comments implicitly
    terminate directive statements
    */
    BEGIN(INITIAL);
}
<LINE_COMMENT>. {
    cpp_scan_consume(yyextra, yytext, yyleng);
}

<*>"/*" {
    /* block comments are handled here */
    cpp_scan_consume(yyextra, "/*", 2);
    yy_push_state(BLOCK_COMMENT, yyscanner);
}
<BLOCK_COMMENT>"*/" {
    cpp_scan_consume(yyextra, "*/", 2);
    yy_pop_state(yyscanner);
}
<BLOCK_COMMENT>.|\n {
    cpp_scan_consume(yyextra, yytext, yyleng);
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

<DIRECTIVE>"include" { BEGIN(INCLUDE); return TOK_INCLUDE; }
<DIRECTIVE>"define" { BEGIN(DEFINE); return TOK_DEFINE; }
<DIRECTIVE>"undef" { return TOK_UNDEF; }
<DIRECTIVE>"if" { return TOK_IF; }
<DIRECTIVE>"ifdef" { return TOK_IFDEF; }
<DIRECTIVE>"ifndef" { return TOK_IFNDEF; }
<DIRECTIVE>"elif" { return TOK_ELIF; }
<DIRECTIVE>"else" { return TOK_ELSE; }
<DIRECTIVE>"endif" { return TOK_ENDIF; }
<DIRECTIVE>"line" { return TOK_LINE; }
<DIRECTIVE>"pragma" { return TOK_PRAGMA; }
<DIRECTIVE>"error" { return TOK_ERROR; }
<DIRECTIVE>"warning" { return TOK_WARNING; }

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

<DEFINE>. {
    BEGIN(DIRECTIVE);
}

<DIRECTIVE,INCLUDE>. {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext, yyleng, extra->instance->arena);
    return TOK_PP_PASTE;
}

"(" { return TOK_LPAREN; }
")" { return TOK_RPAREN; }
"," { return TOK_COMMA; }

<INITIAL>.|\n {
    /* all remaining text should be copied verbatim to the output */
    cpp_scan_consume(yyextra, yytext, yyleng);
}

<<EOF>> {
    if (cpp_leave_file(yyscanner))
        yyterminate();
}

%%

FLEX_MEMORY(cppalloc, cpprealloc, cppfree)
