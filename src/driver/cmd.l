%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="cmd"

%{
#include "cmd-bison.h"
#include "cthulhu/util/report-ext.h"
#include "cthulhu/ast/interop.h"
%}

%%

[ \r\n\t] ;

\-\-[a-zA-Z][a-zA-Z0-9]* {
    yylval->ident = ctu_strdup(yytext + 2); 
    return LONG_OPT; 
}
\-[a-zA-Z][a-zA-Z0-9]* { 
    yylval->ident = ctu_strdup(yytext + 1);
    return SHORT_OPT; 
}

"enable" { return ENABLE; }
"disable" { return DISABLE; }

[a-zA-Z][a-zA-Z0-9]* { 
    yylval->ident = ctu_strdup(yytext);
    return IDENT; 
}

[0-9]+ { 
    return NUMBER; 
}

. { report_unknown_character(yyextra->reports, node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(pl0alloc, pl0realloc, pl0free)
