%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="json"

%{
#include "json_bison.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "cthulhu/events/events.h"
%}

WS [ \t\r\v\n\f]
ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))

%%

{WS}+ ;
"//".* ;

"[" { return LBRACKET; }
"]" { return RBRACKET; }
"{" { return LBRACE; }
"}" { return RBRACE; }
":" { return COLON; }
"," { return COMMA; }

"true" { yylval->boolean = true; return BOOLEAN; }
"false" { yylval->boolean = false; return BOOLEAN; }
"null" { return NULLVAL; }

[0-9]+ { json_scan_integer(yylval->integer, yyextra, yylloc, yytext, yyleng, 10); return INTEGER; }
"0"[0-7]* { json_scan_integer(yylval->integer, yyextra, yylloc, yytext, yyleng, 8); return INTEGER; }
"0"[bB][01]+ { json_scan_integer(yylval->integer, yyextra, yylloc, yytext + 2, yyleng - 2, 2); return INTEGER; }
"0"[xX][0-9a-fA-F]+ { json_scan_integer(yylval->integer, yyextra, yylloc, yytext + 2, yyleng - 2, 16); return INTEGER; }

\"([^"\\\n]|{ESCAPES})*\" { json_scan_string(&yylval->string, yyextra, yylloc, yytext + 1, yyleng - 2); return STRING; }

. {
    json_scan_t *scan = json_scan_context(yyextra);
    evt_scan_unknown(scan->reports, node_new(yyextra, *yylloc), yytext);
}

%%

FLEX_MEMORY(json)
