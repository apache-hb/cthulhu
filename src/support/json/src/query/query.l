%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="query"

%{
#include "query_bison.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "cthulhu/events/events.h"
%}

WS [ \t\r\v\n\f]
ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))

%%

{WS}+ ;

"[" { return LBRACKET; }
"]" { return RBRACKET; }
"." { return DOT; }

[-]?"0"[0-7]* { query_parse_integer(yylval->integer, yyextra, *yylloc, yytext, 8); return INTEGER; }
[-]?"0"[bB][01]+ { query_parse_integer(yylval->integer, yyextra, *yylloc, yytext + 2, 2); return INTEGER; }
[-]?"0"[xX][0-9a-fA-F]+ { query_parse_integer(yylval->integer, yyextra, *yylloc, yytext + 2, 16); return INTEGER; }
[-]?[0-9]+ { query_parse_integer(yylval->integer, yyextra, *yylloc, yytext, 10); return INTEGER; }

[a-zA-Z_][a-zA-Z0-9_]* { query_parse_string(&yylval->string, yyextra, *yylloc, yytext, yyleng); return IDENT; }
\"([^"\\\n]|{ESCAPES})*\" { query_parse_string(&yylval->string, yyextra, *yylloc, yytext + 1, yyleng - 2); return STRING; }

. {
    query_scan_t *scan = query_scan_context(yyextra);
    evt_scan_unknown(scan->reports, node_new(yyextra, *yylloc), yytext);
}

%%

FLEX_MEMORY(query)
