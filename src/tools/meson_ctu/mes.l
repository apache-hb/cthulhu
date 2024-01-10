%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="mes"

%{
#include "mes_bison.h"
#include "mes/scan.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "memory/arena.h"
#include "cthulhu/events/events.h"
%}

%x COMMENT

WS [ \t\r\v\n\f]

%%

. {

}

%%

FLEX_MEMORY(mes)
