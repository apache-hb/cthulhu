%option extra-type="scan_t"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="gen"

%{
#include "gen-bison.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}

%%

[ \t\r\n] ;

"config" { return CONFIG; }
"tokens" { return TOKENS; }
"grammar" { return GRAMMAR; }
"tree" { return TREE; }

":=" { return ASSIGN; }

"{" { return LBRACE; }
"}" { return RBRACE; }

[a-zA-Z_-][a-zA-Z0-9_-]* { yylval->str = ctu_strdup(yytext); return IDENT; }

. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(genalloc, genrealloc, genfree)
