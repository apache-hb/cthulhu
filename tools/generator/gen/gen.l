%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="gen"

%{
#include "gen-bison.h"
%}

%%

[ \r\n\t] ;
"//".* ;

"config" { return CONFIG; }

"line-comment" { return LINE_COMMENT; }
"block-comment" { return BLOCK_COMMENT; }

"keywords" { return KEYWORDS; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->text = ctu_strdup(yytext);
    return IDENT;
}

\"(\\.|[^"\\])*\" { 
    yylval->text = ctu_strndup(yytext + 1, yyleng - 2);
    return STRING;
}

"{" { return LBRACE; }
"}" { return RBRACE; }

"(" { return LPAREN; }
")" { return RPAREN; }

. { report_unknown_character(yyextra->reports, node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(genalloc, genrealloc, genfree)
