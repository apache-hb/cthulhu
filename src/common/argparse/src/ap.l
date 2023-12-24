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

LETTER [a-zA-Z]
OPT [a-zA-Z0-9\-\:]

%%

[ \r\n\t\f] ;

"=" { return ASSIGN; }

[-]{LETTER}{OPT}* {
    /* short opt */
    ap_t *ap = scan_get_context(yyextra);
    return ap_get_opt(ap, yytext + 1, &yylval->field, &yylval->error);
}

[-][-]{LETTER}{OPT}* {
    /* long opt */
    ap_t *ap = scan_get_context(yyextra);
    return ap_get_opt(ap, yytext + 2, &yylval->field, &yylval->error);
}

[/]{LETTER}{OPT}* {
    /* nt style opt */
    ap_t *ap = scan_get_context(yyextra);
    return ap_get_opt(ap, yytext + 1, &yylval->field, &yylval->error);
}

[0-9]+ {
    mpz_init_set_str(yylval->number, yytext, 10);
    return NUMBER;
}

"0x"[0-9a-fA-F]+ {
    mpz_init_set_str(yylval->number, yytext + 2, 16);
    return NUMBER;
}

"0b"[01]+ {
    mpz_init_set_str(yylval->number, yytext + 2, 2);
    return NUMBER;
}

[a-zA-Z0-9_.,/;:\\\-]+ {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

\"(\\.|[^"\\])*\" {
    yylval->ident = ctu_strndup(yytext + 1, yyleng - 2);
    return IDENT;
}

. { ap_on_error(yyextra, yytext); }

%%

FLEX_MEMORY(apalloc, aprealloc, apfree)
