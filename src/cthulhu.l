%option noyywrap
%option never-interactive
%option noinput
%option nounput

%{
#include <stdio.h>

#define YY_DECL int yylex()

#include "cthulhu.tab.hpp"

%}

%%

[ \r\n\t] ;
[0-9]+ { yylval.digit = atoll(yytext); return T_DIGIT; }
"def" { return T_DEF; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval.ident = strdup(yytext); return T_IDENT; }
";" { return T_SEMI; }
"+" { return T_ADD; }
"-" { return T_SUB; }
"*" { return T_MUL; }
"/" { return T_DIV; }
"%" { return T_REM; }
"(" { return T_LPAREN; }
")" { return T_RPAREN; }
"{" { return T_LBRACE; }
"}" { return T_RBRACE; }
. { fprintf(stderr, "unknown characters `%s`\n", yytext); return T_SEMI; }

%%
