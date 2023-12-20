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
%}

LETTER [a-zA-Z]
OPT [a-zA-Z0-9\-\:]

%%

[ \r\n\t] ;

"=" { return ASSIGN; }

[-]+{LETTER}{OPT}* {
    return ap_get_opt(scan_get(yyextra), yytext, &yylval->option, &yylval->error);
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
    yylval->ident = ctu_strdup(yytext, ctu_default_alloc());
    return IDENT;
}

\"(\\.|[^"\\])*\" {
    yylval->ident = ctu_strndup(yytext + 1, yyleng - 2, ctu_default_alloc());
    return IDENT;
}

. { ap_on_error(yyextra, *yylloc, yytext); }

%%

FLEX_MEMORY(apalloc, aprealloc, apfree)
