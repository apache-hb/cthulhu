%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant stack bison-bridge
%option never-interactive batch
%option prefix="cpp"

%{
#define CTU_DISABLE_FLEX_ACTION
#define CTU_DISABLE_FLEX_DEFAULT
#define CTU_DISBALE_FLEX_INIT

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

%%

<*>"//" {
    /* single line comments are handled here */
    cpp_scan_consume(yyextra, yytext, yyleng);
    yy_push_state(LINE_COMMENT, yyscanner);
}
<LINE_COMMENT>\n {
    cpp_scan_consume(yyextra, yytext, yyleng);
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

<LINE_COMMENT,DIRECTIVE,INCLUDE>\\\n { /* a backslash followed by a newline is a line continuation */ }
<DIRECTIVE,INCLUDE>[ \t\r\v\f]+ { /* consume all other (not newline) whitespace */ }

<DIRECTIVE,INCLUDE><<EOF>> {
    /* if we've reached the end of the file then the directive is complete */
    BEGIN(INITIAL);

    if (cpp_leave_file(yyscanner))
        yyterminate();
}

<DIRECTIVE,INCLUDE>\n {
    /* if we've reached an unescaped newline then the directive is complete */
    BEGIN(INITIAL);
}

<DIRECTIVE>"include" { BEGIN(INCLUDE); return TOK_INCLUDE; }
<DIRECTIVE>"define" { return TOK_DEFINE; }
<DIRECTIVE>"undef" { return TOK_UNDEF; }
<DIRECTIVE>"pragma" { return TOK_PRAGMA; }
<DIRECTIVE>"error" { return TOK_ERROR; }

<DIRECTIVE>[a-zA-Z0-9_]+ {
    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext, yyleng, extra->instance->arena);
    return TOK_IDENT;
}

<INCLUDE>[a-zA-Z0-9_]+ {
    BEGIN(INITIAL);

    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext, yyleng, extra->instance->arena);
    return TOK_IDENT;
}

<INCLUDE>"\""[^"\n]*"\"" {
    BEGIN(INITIAL);

    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext + 1, yyleng - 2, extra->instance->arena);
    return TOK_STRING;
}

<INCLUDE>"<"[^>\n]*">" {
    BEGIN(INITIAL);

    cpp_scan_t *extra = cpp_scan_context(yyextra);
    yylval->text = arena_strndup(yytext + 1, yyleng - 2, extra->instance->arena);
    return TOK_SYSTEM;
}

<DIRECTIVE,INCLUDE>. {

}

.|\n {
    /* all remaining text should be copied verbatim to the output */
    cpp_scan_consume(yyextra, yytext, yyleng);
}

<<EOF>> {
    if (cpp_leave_file(yyscanner))
        yyterminate();
}

%%

FLEX_MEMORY(cppalloc, cpprealloc, cppfree)
