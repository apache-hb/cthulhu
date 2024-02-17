%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="meta"

%{
#include "meta_bison.h"
#include "arena/arena.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "cthulhu/events/events.h"
%}

WS [ \t\r\v\n\f]

%%

{WS}+ ;

"config" { return TOK_CONFIG; }
"ast" { return TOK_AST; }
"extends" { return TOK_EXTENDS; }
"abstract" { return TOK_ABSTRACT; }

"opaque" { return TOK_OPAQUE; }
"vector" { return TOK_VECTOR; }
"cstring" { return TOK_CSTRING; }
"mpz" { return TOK_MPZ; }

"=" { return TOK_ASSIGN; }
":" { return TOK_COLON; }
";" { return TOK_SEMICOLON; }
"{" { return TOK_LBRACE; }
"}" { return TOK_RBRACE; }
"(" { return TOK_LPAREN; }
")" { return TOK_RPAREN; }
"[" { return TOK_LBRACKET; }
"]" { return TOK_RBRACKET; }

[a-zA-Z_][a-zA-Z0-9_]* {
    meta_scan_t *scan = meta_scan_context(yyextra);
    yylval->ident = arena_strndup(yytext, yyleng, scan->arena);
    return TOK_IDENT;
}

\"(\\.|[^"])*\" {
    meta_scan_t *scan = meta_scan_context(yyextra);
    yylval->ident = arena_strndup(yytext + 1, yyleng - 2, scan->arena);
    return TOK_STRING;
}

. {
    meta_scan_t *scan = meta_scan_context(yyextra);
    evt_scan_unknown(scan->logger, node_new(yyextra, *yylloc), yytext);
}

%%

FLEX_MEMORY(meta)
