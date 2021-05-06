%option outfile="flex.c" header-file="flex.h"

%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations

%{
#include <stdio.h>
#include "bison.h"

#define YY_USER_ACTION                                             \
    yylloc->first_line = yylloc->last_line;                          \
    yylloc->first_column = yylloc->last_column;                      \
    if (yylloc->last_line == yylineno)                               \
      yylloc->last_column += yyleng;                                 \
    else {                                                           \
      yylloc->last_line = yylineno;                                  \
      yylloc->last_column = yytext + yyleng - strrchr(yytext, '\n'); \
    }
%}

%%

[ \r\n\t] ;
[0-9]+ { yylval->text = strdup(yytext); return DIGIT; }
"def" { return DEF; }
"return" { return RETURN; }
"var" { return VAR; }
"true" { return BOOL_TRUE; }
"false" { return BOOL_FALSE; }
"export" { return OPEN; }
"final" { return FINAL; }
\"(\\.|[^"\\])*\" { yylval->text = strdup(yytext); return STRING; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }
";" { return SEMI; }
"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
"%" { return REM; }
"?" { return QUESTION; }
":" { return COLON; }
"," { return COMMA; }
"=" { return ASSIGN; }
"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }
"&" { return BITAND; }
"!" { return NOT; }
"->" { return ARROW; }
<<EOF>> { return END; }
. { fprintf(stderr, "unknown characters `%s`\n", yytext); }

%%
