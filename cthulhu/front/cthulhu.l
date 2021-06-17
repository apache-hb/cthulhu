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
#include "cthulhu/front/scanner.h"
#include "cthulhu/util/report.h"

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
    result = flex_provide(yyextra, buf); \
    if (result <= 0) result = YY_NULL; 
%}

%%

[ \r\n\t] ;
"//".* ;

"def" { return DEF; }
"final" { return FINAL; }
"return" { return RETURN; }
"true" { return BTRUE; }
"false" { return BFALSE; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }

"0x"[0-9a-fA-F]+ { yylval->digit.text = strdup(yytext + 2); yylval->digit.base = 16; return DIGIT; }
"0b"[01]+ { yylval->digit.text = strdup(yytext + 2); yylval->digit.base = 2; return DIGIT; }
[0-9]+ { yylval->digit.text = strdup(yytext); yylval->digit.base = 10; return DIGIT; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }

"=" { return ASSIGN; }
":" { return COLON; }
"?" { return QUESTION; }
";" { return SEMI; }

"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return REM; }

. { add_lexer_error(format("unknown character `%s`", yytext), yyextra, *yylloc); }

%%
