%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="cc"

%{
#include "cc_bison.h"
#include "cc/scan.h"
#include "interop/flex.h"
#include "memory/memory.h"
#include "cthulhu/util/text.h"
#include "cthulhu/events/events.h"
%}

ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS [ \t\r\v\n\f]

%x COMMENT

%%

{WS} ;
"//".* ;
"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>.|\n ;

"*=" { return MULEQ; }
"*" { return MUL; }

"." { return DOT; }

"::" { return COLON2; }
":" { return COLON; }

";" { return SEMICOLON; }

"void" { return VOID; }
"_Bool" { return BOOL; }

"char" { return CHAR; }
"short" { return SHORT; }
"int" { return INT; }
"long" { return LONG; }

"float" { return FLOAT; }
"double" { return DOUBLE; }

"signed" { return SIGNED; }
"unsigned" { return UNSIGNED; }

"_Atomic" { return ATOMIC; }
"const" { return CONST; }
"volatile" { return VOLATILE; }
"restrict" { return RESTRICT; }
"auto" { return AUTO; }
"extern" { return EXTERN; }
"inline" { return INLINE; }
"static" { return STATIC; }
"register" { return REGISTER; }

"struct" { return STRUCT; }
"union" { return UNION; }
"enum" { return ENUM; }

"break" { return BREAK; }
"case" { return CASE; }
"continue" { return CONTINUE; }
"default" { return DEFAULT; }
"do" { return DO; }
"else" { return ELSE; }
"for" { return FOR; }
"goto" { return GOTO; }
"if" { return IF; }
"switch" { return SWITCH; }
"return" { return RETURN; }

"sizeof" { return SIZEOF; }
"typedef" { return TYPEDEF; }

"while" { return WHILE; }
"_Alignas" { return ALIGNAS; }
"_Alignof" { return ALIGNOF; }
"_Complex" { return COMPLEX; }
"_Generic" { return GENERIC; }
"_Imaginary" { return IMAGINARY; }
"_Noreturn" { return NRETURN; }
"_Static_assert" { return STATIC_ASSERT; }
"_Thread_local" { return THREAD_LOCAL; }

"_Module" { return MODULE; }
"_Import" { return IMPORT; }

"0"[0-7]+ {
    mpz_init_set_str(yylval->mpz, yytext + 1, 8);
    return DIGIT;
}

[0-9]+ {
    mpz_init_set_str(yylval->mpz, yytext, 10);
    return DIGIT;
}

"0"[xX][0-9a-fA-F]+ {
    mpz_init_set_str(yylval->mpz, yytext + 2, 16);
    return DIGIT;
}

"0"[bB][01]+ {
    mpz_init_set_str(yylval->mpz, yytext + 2, 2);
    return DIGIT;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

\"([^"\\\n]|{ESCAPES})*\" {
    cc_scan_t *scan = cc_scan_context(yyextra);
	yylval->text = util_text_escape(scan->reports, node_new(yyextra, *yylloc), yytext + 1, yyleng - 2);
    return STRING;
}

. {
    cc_scan_t *scan = cc_scan_context(yyextra);
    evt_scan_unknown(scan->reports, node_new(yyextra, *yylloc), yytext);
}

%%

FLEX_MEMORY(cc)
