%option outfile="flex.c" header-file="flex.h"

%option noyywrap noinput nounput
%option reentrant bison-bridge

%{
#include <stdio.h>
#include "bison.h"
%}

%%

[ \r\n\t] ;
[0-9]+ { yylval->text = strdup(yytext); return DIGIT; }
"def" { return DEF; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }
";" { return SEMI; }
"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
"%" { return REM; }
"?" { return QUESTION; }
":" { return COLON; }
"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }
<<EOF>> { return END; }
. { fprintf(stderr, "unknown characters `%s`\n", yytext); }

%%
