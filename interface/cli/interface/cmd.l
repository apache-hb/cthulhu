%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="cmd"

%{
#include "cmd-bison.h"
#include "cthulhu/report/report-ext.h"
#include "cthulhu/ast/interop.h"
%}

LETTER [a-zA-Z]
ALNUMDASH [a-zA-Z0-9-]

%%

[ \r\n\t] ;

"=" { return ASSIGN; }

[-]+{LETTER}{ALNUMDASH}* {
    yylval->ident = ctu_strdup(yytext); 
    return OPT; 
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

. { report_unknown_character(yyextra->reports, node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(cmdalloc, cmdrealloc, cmdfree)
