%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="cc"

%{
#include "cc-bison.h"
#include "scan.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}

ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS [ \t\r\v\n\f]

%%

{WS} ;
"//".* ;

"auto" { return AUTO; }
"break" { return BREAK; }
"case" { return CASE; }
"char" { return CHAR; }
"const" { return CONST; }
"continue" { return CONTINUE; }
"default" { return DEFAULT; }
"do" { return DO; }
"double" { return DOUBLE; }
"else" { return ELSE; }
"enum" { return ENUM; }

"extern" { return EXTERN; }
"float" { return FLOAT; }
"for" { return FOR; }
"goto" { return GOTO; }
"if" { return IF; }
"inline" { return INLINE; }
"int" { return INT; }
"long" { return LONG; }
"register" { return REGISTER; }
"restrict" { return RESTRICT; }
"return" { return RETURN; }

"short" { return SHORT; }
"signed" { return SIGNED; }
"sizeof" { return SIZEOF; }
"static" { return STATIC; }
"struct" { return STRUCT; }
"switch" { return SWITCH; }
"typedef" { return TYPEDEF; }
"union" { return UNION; }
"unsigned" { return UNSIGNED; }
"void" { return VOID; }
"volatile" { return VOLATILE; }

"while" { return WHILE; }
"_Alignas" { return ALIGNAS; }
"_Alignof" { return ALIGNOF; }
"_Atomic" { return ATOMIC; }
"_Bool" { return BOOL; }
"_Complex" { return COMPLEX; }
"_Generic" { return GENERIC; }
"_Imaginary" { return IMAGINARY; }
"_Noreturn" { return NORETURN; }
"_Static_assert" { return STATIC_ASSERT; }
"_Thread_local" { return THREAD_LOCAL; }

"0"[0-7]+ {
    mpz_init_set_str(yylval->digit, yytext + 1, 8);
    return DIGIT;
}

[0-9]+ {
    mpz_init_set_str(yylval->digit, yytext, 10);
    return DIGIT;
}

"0x"[0-9a-fA-F]+ {
    mpz_init_set_str(yylval->digit, yytext + 2, 16);
    return DIGIT;
}

"0b"[01]+ {
    mpz_init_set_str(yylval->digit, yytext + 2, 2);
    return DIGIT;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

\"([^"\\\n]|{ESCAPES})*\" {
	yylval->string.data = ctu_strndup(yytext + 1, yyleng - 2);
    yylval->string.length = yyleng - 2;
    return STRING;
}

. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(ccalloc, ccrealloc, ccfree)
