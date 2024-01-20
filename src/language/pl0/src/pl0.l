%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="pl0"

%{
#include "pl0_bison.h"
#include "pl0/scan.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "cthulhu/events/events.h"
%}

%x COMMENT

WS [ \t\r\v\n\f]

%%

{WS} ;
"//".* ;
"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>.|\n ;

(?i:module) { return MODULE; }
(?i:import) { return IMPORT; }
(?i:const) { return CONST; }
(?i:var) { return VAR; }
(?i:procedure) { return PROCEDURE; }

(?i:begin) { return START; }
(?i:end) { return END; }
(?i:if) { return IF; }
(?i:then) { return THEN; }
(?i:while) { return WHILE; }
(?i:do) { return DO; }
(?i:call) { return CALL; }

(?i:odd) { return ODD; }

[a-zA-Z_][a-zA-Z0-9_]* {
    arena_t *arena = scan_get_arena(yyextra);
    yylval->ident = arena_strdup(yytext, arena);
    return IDENT;
}

[0-9]+ {
    mpz_init_set_str(yylval->number, yytext, 10);
    return NUMBER;
}

";" { return SEMICOLON; }
"." { return DOT; }
"," { return COMMA; }
":=" { return ASSIGN; }

"=" { return EQUALS; }
"!=" { return NOTEQUAL; }
"<" { return LESS; }
">" { return GREATER; }
"<=" { return LESSEQ; }
">=" { return GREATEQ; }

"(" { return LPAREN; }
")" { return RPAREN; }

"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }

"!" { return PRINT; }

. {
    pl0_scan_t *scan = pl0_scan_context(yyextra);
    evt_scan_unknown(scan->reports, node_new(yyextra, *yylloc), yytext);
}

%%

FLEX_MEMORY(pl0)
