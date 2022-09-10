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

"struct" { return STRUCT; }
"union" { return UNION; }

"auto" { return AUTO; }
"break" { return BREAK; }
"case" { return CASE; }
"continue" { return CONTINUE; }
"default" { return DEFAULT; }
"do" { return DO; }
"else" { return ELSE; }
"enum" { return ENUM; }

"extern" { return EXTERN; }
"for" { return FOR; }
"goto" { return GOTO; }
"if" { return IF; }
"inline" { return INLINE; }
"register" { return REGISTER; }
"restrict" { return RESTRICT; }
"return" { return RETURN; }

"sizeof" { return SIZEOF; }
"static" { return STATIC; }
"switch" { return SWITCH; }
"typedef" { return TYPEDEF; }

"while" { return WHILE; }
"_Alignas" { return ALIGNAS; }
"_Alignof" { return ALIGNOF; }
"_Complex" { return COMPLEX; }
"_Generic" { return GENERIC; }
"_Imaginary" { return IMAGINARY; }
"_Noreturn" { return NORETURN; }
"_Static_assert" { return STATIC_ASSERT; }
"_Thread_local" { return THREAD_LOCAL; }

"virtual" { return VIRTUAL; }

"0"[0-7]+ {
    mpz_init_set_str(yylval->digit, yytext + 1, 8);
    return DIGIT;
}

[0-9]+ {
    mpz_init_set_str(yylval->digit, yytext, 10);
    return DIGIT;
}

"0"[xX][0-9a-fA-F]+ {
    mpz_init_set_str(yylval->digit, yytext + 2, 16);
    return DIGIT;
}

"0"[bB][01]+ {
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