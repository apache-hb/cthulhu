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
"else" { return ELSE; }
"as" { return AS; }
"final" { return FINAL; }
"var" { return VAR; }
"while" { return WHILE; }
"true" { return BOOL_TRUE; }
"false" { return BOOL_FALSE; }
"return" { return RETURN; }
"export" { return EXPORTED; }

"$" { yylval->text = "$"; return IDENT; }
[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }

[0-9]+ { 
    yylval->digit.text = yytext; 
    yylval->digit.base = 10;
    return DIGIT; 
}

"0b"[01]+ { 
    yylval->digit.text = yytext + 2; 
    yylval->digit.base = 2;
    return DIGIT; 
}

"0x"[0-9a-fA-F]+ { 
    yylval->digit.text = yytext + 2;
    yylval->digit.base = 16;
    return DIGIT; 
}


">=" { return GTE; }
"<=" { return LTE; }
">" { return GT; }
"<" { return LT; }
"==" { return EQ; }
"!=" { return NEQ; }

"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return REM; }

"&" { return BITAND; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }
"?" { return QUESTION; }
";" { return SEMI; }
":" { return COLON; }
"," { return COMMA; }
"=" { return ASSIGN; }

\0 { return END; }

. { report(LEVEL_ERROR, yyextra, *yylloc, "unknown character `%s`", yytext); }

%%
