%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="cc"

%{
#include "cc-bison.h"
#include "scan.h"
#include "ctu/util/report.h"
#include "ctu/ast/interop.h"
%}

%%

[ \r\n\t] ;

"const" { return CONST; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

[0]|[1-9][0-9]+ {
    mpz_init_set_str(yylval->integer, yytext, 10);
    return INTEGER;
}

. { report2(yyextra->reports, ERROR, node_new(yyextra, *yylloc),"unknown character %s", yytext); }

%%
