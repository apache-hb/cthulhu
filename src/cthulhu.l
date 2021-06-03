%option outfile="flex.c" header-file="flex.h"

%option extra-type="scanner_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations

%{
#include <string.h>
#include "bison.h"

#define YY_USER_ACTION                                               \
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
"//".* ;

[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }
[0-9]+ { yylval->text = strdup(yytext); return DIGIT; }

"(" { return LPAREN; }
")" { return RPAREN; }

":" { return COLON; }
"?" { return QUESTION; }
";" { return SEMI; }
"," { return COMMA; }

"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return REM; }

. { fprintf(stderr, "unknown character `%s`\n", yytext); }

%%
