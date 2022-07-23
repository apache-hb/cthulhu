%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="gen"

%{
#include "gen-bison.h"
#include "gen.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}

WS [ \t\r\v\n\f]

%%

{WS} ;
"//".* ;

":" { return COLON; }

"{" { return LBRACE; }
"}" { return RBRACE; }

"(" { return LPAREN; }
")" { return RPAREN; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->text = ctu_strdup(yytext);
    return IDENT;
}

\"([^"\\\n])*\" {
	yylval->string.data = ctu_strndup(yytext + 1, yyleng - 2);
    yylval->string.length = yyleng - 2;
    return STRING;
}


. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(pl0alloc, pl0realloc, pl0free)
