%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="gen"

%{
#include "gen-bison.h"
#include "base/util.h"
#include "cthulhu/ast/interop.h"
#include "cthulhu/report/report-ext.h"
%}

%%

[ \r\n\t] ;
"//".* ;

"{{"(^"}}"|[^\\])*"}}" {
    /* god knows how exactly this regex works */
    /* this doesnt like when \\ is in the middle of a codeblock */
    /* but C doesnt have that so who cares lol */
    yylval->text = ctu_strndup(yytext + 2, yyleng - 4);
    return CODE;
}

"{" { return LBRACE; }
"}" { return RBRACE; }

"(" { return LPAREN; }
")" { return RPAREN; }

"[" { return LSQUARE; }
"]" { return RSQUARE; }

"," { return COMMA; }
":=" { return ASSIGN; }
"::" { return COLON2; }

"config" { return CONFIG; }
"lexer" { return LEXER; }
"parser" { return PARSER; }

[a-zA-Z_][a-zA-Z0-9_-]* {
    yylval->text = ctu_strdup(yytext);
    return IDENT;
}

\"(\\.|[^"\\])*\" { 
    /* a string that doesnt include its own quotes in the output */
    yylval->text = ctu_strndup(yytext + 1, yyleng - 2);
    return STRING;
}

"q"\"(\\.|[^"\\])*\" { 
    /* a string that includes its own quotes in the output */
    yylval->text = ctu_strndup(yytext + 1, yyleng - 1);
    return STRING;
}

. { report_unknown_character(yyextra->reports, node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(genalloc, genrealloc, genfree)
