%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="ap"

%{
#include "ap_bison.h"
#include "interop/flex.h"
#include "memory/memory.h"
#include "scan/node.h"
%}

FLAG_BEGIN [a-zA-Z?]
FLAG_TEXT [a-zA-Z0-9\-\?]

%%

[ \r\n\t\f] ;

"=" { return ASSIGN; }
":" { return ASSIGN; }

[-]{FLAG_BEGIN}{FLAG_TEXT}* {
    return ap_get_opt(yyextra, yytext + 1, &yylval->field, &yylval->error);
}

[-][-]{FLAG_BEGIN}{FLAG_TEXT}* {
    return ap_get_opt(yyextra, yytext + 2, &yylval->field, &yylval->error);
}

[/]{FLAG_BEGIN}{FLAG_TEXT}* {
    return ap_get_opt(yyextra, yytext + 1, &yylval->field, &yylval->error);
}

[0-9]+ {
    mpz_init_set_str(yylval->number, yytext, 10);
    return NUMBER;
}

"0"[xX][0-9a-fA-F]+ {
    mpz_init_set_str(yylval->number, yytext + 2, 16);
    return NUMBER;
}

"0"[bB][01]+ {
    mpz_init_set_str(yylval->number, yytext + 2, 2);
    return NUMBER;
}

[a-zA-Z0-9_.,/;\\\-]+ {
    ap_t *self = scan_get_context(yyextra);
    yylval->ident = arena_strdup(yytext, self->arena);
    return IDENT;
}

\"(\\.|[^"\\])*\" {
    ap_t *self = scan_get_context(yyextra);
    yylval->ident = arena_strndup(yytext + 1, yyleng - 2, self->arena);
    return IDENT;
}

. { ap_on_invalid(yyextra, yytext); }

%%

FLEX_MEMORY(apalloc, aprealloc, apfree)
