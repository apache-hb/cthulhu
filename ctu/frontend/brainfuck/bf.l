%option extra-type="scan_t*"
%option yylineno 8bit
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="bf"

%{
#include "bf-bison.h"
#include "scan.h"
#include "ctu/util/report.h"
#include "ctu/util/report-ext.h"
#include "ctu/ast/interop.h"
%}

%%

"+" { return ADD; }
"-" { return SUB; }
">" { return INC; }
"<" { return DEC; }
"." { return OUT; }
"," { return IN; }
"[" { return LBRACKET; }
"]" { return RBRACKET; }

%%

FLEX_MEMORY(bfalloc, bfrealloc, bffree)
