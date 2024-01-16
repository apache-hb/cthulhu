%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant
%option never-interactive batch
%option prefix="pre"

%{
#include "pre/scan.h"
#include "interop/memory.h"
%}

WS [ \t\r\v\n\f]

%x COMMENT

%%

{WS} ;
"//".* ;
"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>.|\n ;

. {
    cpp_scan_t *scan = cpp_scan_context(yyextra);
    where_t where = { 0 };
    evt_scan_unknown(scan->reports, node_new(yyextra, where), yytext);
}

%%

FLEX_MEMORY(pre)
