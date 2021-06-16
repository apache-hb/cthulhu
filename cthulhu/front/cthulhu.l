%option outfile="flex.c" header-file="flex.h"

%option extra-type="scanner_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations
%option never-interactive batch

%{
#include <string.h>
#include <stdint.h>
#include "bison.h"

#include "cthulhu/util/report.h"

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

#define YY_INPUT(buf, result, size) \
    result = flex_provide(yylloc, yyextra, buf); \
    if (result <= 0) result = YY_NULL; 
%}

%%

[ \r\n\t] ;
"//".* ;

"def" { return DEF; }
"return" { return RETURN; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }

[0-9]+ { yylval->text = strdup(yytext); return DIGIT; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }

":" { return COLON; }
"?" { return QUESTION; }
";" { return SEMI; }

"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return REM; }

. { scan_reportf(yylloc, yyextra, "unknown character `%s`", yytext); }

%%
