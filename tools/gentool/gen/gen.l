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
#include "std/str.h"
%}

ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS [ \t\r\v\n\f]

%%

{WS} ;
"//".* ; 

"config" { return CONFIG; }
"tokens" { return TOKENS; }
"grammar" { return GRAMMAR; }
"tree" { return TREE; }

":=" { return ASSIGN; }

"{" { return LBRACE; }
"}" { return RBRACE; }

"(" { return LPAREN; }
")" { return RPAREN; }

"[" { return LSQUARE; }
"]" { return RSQUARE; }

"re"[^\n]+ {
	yylval->str = str_trim(yytext + 2, STR_WHITESPACE);
    return REGEX;
}

[a-zA-Z_-][a-zA-Z0-9_-]* { yylval->str = ctu_strdup(yytext); return IDENT; }

\"([^"\\\n]|{ESCAPES})*\" {
	yylval->str = ctu_strndup(yytext + 1, yyleng - 2);
    return STRING;
}

. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(genalloc, genrealloc, genfree)
