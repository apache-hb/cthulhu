%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="pl0"

%{
#include "pl0-bison.h"
#include "scan.h"
#include "ctu/util/report.h"
#include "ctu/ast/interop.h"
%}

%%

[ \r\n\t] ;

(?i:const) { return CONST; }
(?i:var) { return VAR; }
(?i:procedure) { return PROCEDURE; }

(?i:begin) { return START; }
(?i:end) { return END; }
(?i:if) { return IF; }
(?i:then) { return THEN; }
(?i:while) { return WHILE; }
(?i:do) { return DO; }
(?i:call) { return CALL; }

(?i:odd) { return ODD; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

[0-9]+ {
    mpz_init_set_str(yylval->number, yytext, 10);
    return NUMBER;
}

";" { return SEMICOLON; }
"." { return DOT; }
"," { return COMMA; }
":=" { return ASSIGN; }

"=" { return EQUALS; }
"#" { return NOTEQUAL; }
"<" { return LESS; }
">" { return GREATER; }
"<=" { return LESSEQ; }
">=" { return GREATEQ; }

"(" { return LPAREN; }
")" { return RPAREN; }

"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }

. { reportf(ERROR, yyextra, *yylloc, "unknown character %s", yytext); }

%%
