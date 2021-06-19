%option extra-type="scanner_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations
%option never-interactive batch

%{
#include "bison.h"
#include "ctu/ast/scanner.h"

#define YY_USER_ACTION \
    flex_update(yylloc, yylineno, yyleng, yytext);

#define YY_INPUT(buffer, result, size) \
    result = flex_get(yyextra, buffer); \
    if (result <= 0) { result = YY_NULL; }
    
%}

%%

[ \r\n\t] ;
"//".* ;

[a-zA-Z_][a-zA-Z0-9_]* { return IDENT; }

. { puts("death"); }

%%
