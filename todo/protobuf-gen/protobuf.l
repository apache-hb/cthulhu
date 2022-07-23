%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="pb"

%{
#include "pb-bison.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}

%%

[ \r\n\t] ;

"=" { return ASSIGN; }
";" { return SEMI; }

"syntax" { return SYNTAX; }
"message" { return MESSAGE; }
"repeated" { return REPEATED; }
"optional" { return OPTIONAL; }
"oneof" { return ONEOF; }
"enum" { return ENUM; }

"{" { return LBRACE; }
"}" { return RBRACE; }

[0-9]+ {
    yylval->number = field_id(yyextra, *yylloc, yytext);
    return NUMBER; 
}

[a-zA-Z0-9_]+ { 
    yylval->ident = ctu_strdup(yytext);
    return IDENT; 
}

\"(\\.|[^"\\])*\" { 
    yylval->ident = ctu_strndup(yytext + 1, yyleng - 2);
    return STRING;
}

. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(pballoc, pbrealloc, pbfree)
