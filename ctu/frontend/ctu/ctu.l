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

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

. { reportf(ERROR, yyextra, *yylloc, "unknown character %s", yytext); }

%%
