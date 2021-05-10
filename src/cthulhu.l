%option outfile="flex.c" header-file="flex.h"

%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations

%{
#include <stdio.h>
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
[0-9]+ { yylval->text = strdup(yytext); return DIGIT; }
"0x"[0-9a-fA-F]+ { yylval->text = strdup(yytext); return XDIGIT; }
"0b"[01]+ { yylval->text = strdup(yytext); return BDIGIT; }
\"\"\"(\\.|[^"\\])*\"\"\" { yylval->text = strdup(yytext); return MSTRING; }
\"(\\.|[^"\\])*\" { yylval->text = strdup(yytext); return STRING; }

"null" { return NIL; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }
. { fprintf(stderr, "unknown characters `%s`\n", yytext); }

%%
