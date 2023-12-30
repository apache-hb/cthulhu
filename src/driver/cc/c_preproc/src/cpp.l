%option extra-type="scan_t*"
%option 8bit nodefault
%option noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant stack noyywrap
%option never-interactive batch
%option prefix="cpp"

%{
#define CTU_DISABLE_FLEX_ACTION
#define CTU_DISABLE_FLEX_DEFAULT
#define CTU_DISBALE_FLEX_INIT

#include "base/log.h"
#include "interop/flex.h"
#include "cpp/scan.h"
#include "memory/arena.h"
%}

%x BLOCK_COMMENT
%x LINE_COMMENT
%x DIRECTIVE

/* all directive states */
%x INCLUDE
%x DEFINE
%x PRAGMA
%x ERROR

%%

<*>"//" {
    /* single line comments are handled here */
    cpp_scan_consume(yyextra, yytext, yyleng);
    yy_push_state(LINE_COMMENT, yyscanner);
}

<LINE_COMMENT>\n {
    cpp_scan_consume(yyextra, yytext, yyleng);
    cpp_leave_directive(yyextra);
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
    cpp_enter_directive(yyextra);
    BEGIN(DIRECTIVE);
}

<LINE_COMMENT,DIRECTIVE,INCLUDE,DEFINE,PRAGMA,ERROR>\\\n {
    /* a backslash followed by a newline is a line continuation */
}
<DIRECTIVE,INCLUDE,DEFINE,PRAGMA,ERROR>[ \t\r\v\f]+ {
    /* consume all other (not newline) whitespace */
}

<DIRECTIVE,INCLUDE,DEFINE,PRAGMA,ERROR><<EOF>> {
    /* if we've reached an unescaped newline then the directive is complete */
    cpp_leave_directive(yyextra);
    BEGIN(INITIAL);
}

<DIRECTIVE,INCLUDE,DEFINE,PRAGMA,ERROR>\n {
    /* if we've reached an unescaped newline then the directive is complete */
    cpp_leave_directive(yyextra);
    BEGIN(INITIAL);
}

<DIRECTIVE>"include" { BEGIN(INCLUDE); }
<DIRECTIVE>"define" { BEGIN(DEFINE); }
<DIRECTIVE>"pragma" { BEGIN(PRAGMA); }
<DIRECTIVE>"error" { BEGIN(ERROR); }

<INCLUDE>[a-zA-Z0-9_]+ {
    if (cpp_check_recursion(yyextra, yytext))
        cpp_accept_define_include(yyscanner, yytext);

    BEGIN(INITIAL);
}

<INCLUDE>"\""[^"\n]*"\"" {
    if (cpp_check_recursion(yyextra, yytext))
        cpp_accept_include(yyscanner, yytext + 1, yyleng - 2);

    BEGIN(INITIAL);
}

<INCLUDE>"<"[^>\n]*">" {
    if (cpp_check_recursion(yyextra, yytext))
        cpp_accept_include(yyscanner, yytext + 1, yyleng - 2);

    BEGIN(INITIAL);
}

<INCLUDE>. {
    ctu_log("#include: %s", yytext);
}

<DEFINE>.+ {
    //ctu_log("#define: %s", yytext);
}

<PRAGMA>.+ {
    //ctu_log("#pragma: %s", yytext);
}

<ERROR>.+ {

}

<DIRECTIVE>. { }

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
