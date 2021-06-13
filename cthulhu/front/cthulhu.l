%option outfile="flex.c" header-file="flex.h"

%option extra-type="scanner_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations

%{
#include <string.h>
#include <stdint.h>
#include "bison.h"

#ifdef _MSC_VER
#   define strdup _strdup
#else
static char *strdup(const char *str) {
  size_t len = strlen(str) + 1;
  char *out = malloc(len);
  memcpy(out, str, len);
  return out;
}
#endif

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

"(" { return LPAREN; }
")" { return RPAREN; }

":" { return COLON; }
"?" { return QUESTION; }
";" { return SEMI; }

"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return REM; }

. { fprintf(stderr, "unknown character `%s`\n", yytext); }

%%
