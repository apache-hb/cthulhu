%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="ctu"

%{
#include "ctu-bison.h"
#include "scan.h"
#include "ctu/util/report.h"
#include "ctu/ast/interop.h"
%}

%%

[ \r\n\t] ;

"var" { return VAR; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

[0-9]+ {
    mpz_init_set_str(yylval->digit, yytext, 10);
    return DIGIT;
}

";" { return SEMI; }
"=" { return ASSIGN; }

. { reportf(ERROR, yyextra, *yylloc, "unknown character %s", yytext); }

%%
