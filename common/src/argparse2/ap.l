%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="ap"

%{
#include "ap-bison.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}

LETTER [a-zA-Z]
ALNUMDASH [a-zA-Z0-9-]

%%

[ \r\n\t] ;

"=" { return ASSIGN; }

[-]+{LETTER}{ALNUMDASH}* {
    return ap2_get_opt(scan_extra(yyextra), yytext, &yylval->option); 
}

[0-9]+ {
    mpz_init_set_str(yylval->number, yytext, 10);
    return NUMBER; 
}

"0x"[0-9a-fA-F]+ {
    mpz_init_set_str(yylval->number, yytext + 2, 16);
    return NUMBER;
}

"0b"[01]+ {
    mpz_init_set_str(yylval->number, yytext + 2, 2);
    return NUMBER;
}

[a-zA-Z0-9_./;:\\\-]+ { 
    yylval->ident = ctu_strdup(yytext);
    return IDENT; 
}

\"(\\.|[^"\\])*\" { 
    yylval->ident = ctu_strndup(yytext + 1, yyleng - 2);
    return IDENT;
}

. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(apalloc, aprealloc, apfree)
