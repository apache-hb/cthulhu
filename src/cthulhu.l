%option noyywrap
%option never-interactive
%option noinput
%option nounput

%{
#include <stdio.h>

#define YY_DECL int yylex()

#include "cthulhu.tab.h"

%}

%%

[ \r\n\t] ;
[0-9]+ { yylval.digit = atoll(yytext); return DIGIT; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.ident = strdup(yytext); return IDENT; }
";" { return SEMI; }
"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
"%" { return REM; }
"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }
. { fprintf(stderr, "unknown characters `%s`\n", yytext); return SEMI; }

%%
