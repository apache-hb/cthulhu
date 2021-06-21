%option extra-type="scanner_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations
%option never-interactive batch

%{
#include "bison.h"
#include "ctu/ast/scanner.h"
#include "ctu/util/report.h"
#include "ctu/util/str.h"

#define YY_USER_ACTION flex_update(yylloc, yytext);

#define YY_INPUT(buffer, result, size) \
    result = flex_get(yyextra, buffer, size); \
    if (result <= 0) { result = YY_NULL; }

#define YY_USER_INIT flex_init(yylloc, 0)

%}

%%

[ \r\n\t] ;
"//".* ;

"def" { return DEF; }
"if" { return IF; }
"return" { return RETURN; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }

[0-9]+ { yylval->text = yytext; return DIGIT; }

"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return REM; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }
"?" { return QUESTION; }
";" { return SEMI; }

\0 { return END; }

. { report(LEVEL_ERROR, yyextra, *yylloc, "unknown character `%s`", yytext); }

%%
