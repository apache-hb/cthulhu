%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="sql"

%{
#include "sql_bison.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "cthulhu/events/events.h"
%}

WS [ \t\r\v\n\f]

%%

{WS}+ ;

. {
    sql_scan_t *scan = sql_scan_context(yyextra);
    evt_scan_unknown(scan->reports, node_new(yyextra, *yylloc), yytext);
}

%%

FLEX_MEMORY(sql)
