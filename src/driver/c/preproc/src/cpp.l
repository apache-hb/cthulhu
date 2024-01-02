%option extra-type="cpp_extra_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch stack
%option prefix="cpp"

%{
#include "cpp_bison.h"
#include "cpp/scan.h"
#include "base/log.h"
%}

%x BLOCK_COMMENT
%x LINE_COMMENT

/* directive is split up between DIRECTIVE, BODY, and INCLUDE states
 * BODY is to prevent DIRECTIVE keywords from being passed to bison
 * INCLUDE is because we do includes in the lexer for better location tracking
 */
%x DIRECTIVE
%x BODY
%x INCLUDE

WS [ \t\r\v\f]
NL (\n|\r\n)
LC \\{NL}

%%

"\""[^\n]*"\"" {
    /* TODO: pass to bison */
    yylval->text = cpp_text_new(yyextra, yytext, yyleng);
    return TOK_STRING;
}

\'\[^\n]*\' {
    /* TODO: pass to bison */
    yylval->text = cpp_text_new(yyextra, yytext, yyleng);
    return TOK_STRING;
}

<INITIAL>"#" { BEGIN(DIRECTIVE); return TOK_BEGIN_DIRECTIVE; }
<DIRECTIVE>"include" { BEGIN(INCLUDE); return TOK_INCLUDE; }
<DIRECTIVE>"define" { BEGIN(BODY); return TOK_DEFINE; }
<DIRECTIVE>"undef" { BEGIN(BODY); return TOK_UNDEF; }
<DIRECTIVE>"if" { BEGIN(BODY); return TOK_IF; }
<DIRECTIVE>"ifdef" { BEGIN(BODY); return TOK_IFDEF; }
<DIRECTIVE>"ifndef" { BEGIN(BODY); return TOK_IFNDEF; }
<DIRECTIVE>"elif" { BEGIN(BODY); return TOK_ELIF; }
<DIRECTIVE>"else" { BEGIN(BODY); return TOK_ELSE; }
<DIRECTIVE>"endif" { BEGIN(BODY); return TOK_ENDIF; }
<DIRECTIVE>"line" { BEGIN(BODY); return TOK_LINE; }
<DIRECTIVE>"error" { BEGIN(BODY); return TOK_ERROR; }
<DIRECTIVE>"pragma" { BEGIN(BODY); return TOK_PRAGMA; }
<DIRECTIVE>"warning" { BEGIN(BODY); return TOK_WARNING; }

<INITIAL,DIRECTIVE,BODY>[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->text = cpp_text_new(yyextra, yytext, yyleng);
    return TOK_IDENT;
}

<INCLUDE>({LC}|{WS}) { /* ignore all these when parsing includes */ }
<INCLUDE>"<"[^\n]*">" {
    cpp_include_system(yyextra, yytext + 1, yyleng - 2);
    BEGIN(INITIAL);
    return TOK_END_INCLUDE;
}

<INCLUDE>"\""[^\n]*"\"" {
    cpp_include_local(yyextra, yytext + 1, yyleng - 2);
    BEGIN(INITIAL);
    return TOK_END_INCLUDE;
}

<INCLUDE>[a-zA-Z_][a-zA-Z0-9_]* {
    cpp_include_define(yyextra, yytext, yyleng);
    BEGIN(INITIAL);
    return TOK_END_INCLUDE;
}

<INITIAL,DIRECTIVE,INCLUDE,BODY>"//" {
    cpp_push_comment(yyextra, "//", 2);
    BEGIN(LINE_COMMENT);
}
<LINE_COMMENT>\n {
    yylval->text = cpp_reset_comment(yyextra);
    BEGIN(INITIAL);
    return TOK_LINE_COMMENT;
}
<LINE_COMMENT>. {
    cpp_push_comment(yyextra, yytext, yyleng);
}

<INITIAL,DIRECTIVE,INCLUDE,BODY>"/*" {
    cpp_push_comment(yyextra, "/*", 2);
    // use push state here because block comments can appear in directives
    // and not end the directive
    yy_push_state(BLOCK_COMMENT, yyscanner);
}
<BLOCK_COMMENT>"*/" {
    /* TODO: collect comment and pass it to bison */
    cpp_push_comment(yyextra, "*/", 2);
    yy_pop_state(yyscanner);

    yylval->text = cpp_reset_comment(yyextra);
    return TOK_BLOCK_COMMENT;
}
<BLOCK_COMMENT>.|\n {
    cpp_push_comment(yyextra, yytext, yyleng);
}

<LINE_COMMENT,DIRECTIVE,BODY>({LC}|{WS})+ {
    yylval->text = cpp_text_new(yyextra, yytext, yyleng);
    return TOK_WHITESPACE;
}

<DIRECTIVE,BODY>{NL} {
    BEGIN(INITIAL);
    return TOK_END_DIRECTIVE;
}

<*>.|\n {
    /* TODO: assert */
    yylval->paste = *yytext;
    return TOK_PASTE;
}

<<EOF>> {
    if (!cpp_leave_file(yyextra))
        yyterminate();
}

%%

inline void *cppalloc(size_t size, yyscan_t scanner)
{
    cpp_extra_t *extra = cpp_get_scanner_extra(scanner);
    arena_t *arena = cpp_get_scanner_arena(scanner);
    return ARENA_MALLOC(arena, size, "cppalloc", extra);
}

inline void *cpprealloc(void *ptr, size_t size, yyscan_t scanner)
{
    arena_t *arena = cpp_get_scanner_arena(scanner);
    return arena_realloc(ptr, size, ALLOC_SIZE_UNKNOWN, arena);
}

inline void cppfree(void *ptr, yyscan_t scanner)
{
    if (ptr == NULL) return;
    arena_t *arena = cpp_get_scanner_arena(scanner);
    arena_free(ptr, ALLOC_SIZE_UNKNOWN, arena);
}
