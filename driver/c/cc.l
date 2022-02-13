%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="cc"

%{
#include "cc-bison.h"
#include "scan.h"
#include "cthulhu/util/report.h"
#include "cthulhu/util/report-ext.h"
#include "cthulhu/ast/interop.h"
%}

%x COMMENT

%%

[ \r\n\t] ;
"//".* ;
"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n ;
<COMMENT>. ;

"+" { return ADD; }
"-" { return SUB; }
"*" { return STAR; }
"/" { return DIV; }
"%" { return REM; }

"..." { return DOT3; }
"." { return DOT; }

"=" { return ASSIGN; }
":" { return COLON; }
"::" { return COLON2; }
";" { return SEMI; }
"," { return COMMA; }
"?" { return QUESTION; }

"(" { return LPAREN; }
")" { return RPAREN; }

"{" { return LBRACE; }
"}" { return RBRACE; }

"_Module" { return MODULE; }
"_Import" { return IMPORT; }

"_Bool" { return BOOL; }
"void" { return VOID; }

"char" { return CHAR; }
"short" { return SHORT; }
"int" { return INT; }
"long" { return LONG; }

"signed" { return SIGNED; }
"unsigned" { return UNSIGNED; }

"const" { return CONST; }
"volatile" { return VOLATILE; }

"extern" { return EXTERN; }
"static" { return STATIC; }
"auto" { return AUTO; }

[0][0-9]* {
    // octal digit
    mpz_init_set_str(yylval->mpz, yytext, 8);
    return DIGIT;
}

[1-9][0-9]* {
    mpz_init_set_str(yylval->mpz, yytext, 10);
    return DIGIT;
}

"0b"[01]+ { 
    mpz_init_set_str(yylval->mpz, yytext + 2, 2);
    return DIGIT;
}

"0x"[0-9a-fA-F]+ {
    mpz_init_set_str(yylval->mpz, yytext + 2, 16);
    return DIGIT;
}

\"(\\.|[^"\\])*\" { 
    yylval->ident = ctu_strndup(yytext + 1, yyleng - 2);
    return STRING;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

. { report_unknown_character(yyextra->reports, node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(ccalloc, ccrealloc, ccfree)
