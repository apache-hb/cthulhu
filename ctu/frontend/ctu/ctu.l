%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="ctu"

%{
#include "ctu-bison.h"
#include "scan.h"
#include "ctu/util/report.h"
#include "ctu/util/report-ext.h"
#include "ctu/ast/interop.h"
%}

%x COMMENT

%%

[ \r\n\t] ;
"//".* ;
"/$" { BEGIN(COMMENT); }
<COMMENT>"$/" { BEGIN(INITIAL); }
<COMMENT>\n ;
<COMMENT>. ;

"def" { return DEF; }
"var" { return VAR; }
"final" { return FINAL; }
"export" { return EXPORT; }
"return" { return RETURN; }
"if" { return IF; }
"else" { return ELSE; }
"while" { return WHILE; }
"true" { return YES; }
"false" { return NO; }
"as" { return AS; }
"break" { return BREAK; }
"import" { return IMPORT; }
"lambda" { return LAMBDA; }

[$a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

[0-9]+ {
    mpz_init_set_str(yylval->digit, yytext, 10);
    return DIGIT;
}

"0b"[01]+ { 
    mpz_init_set_str(yylval->digit, yytext + 2, 2);
    return DIGIT;
}

"0x"[0-9a-fA-F]+ {
    mpz_init_set_str(yylval->digit, yytext + 2, 16);
    return DIGIT;
}

\"(\\.|[^"\\])*\" { 
    yylval->ident = ctu_strndup(yytext + 1, yyleng - 2);
    return STRING;
}

";" { return SEMI; }

"::" { return COLON2; }
":" { return COLON; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }
"[" { return LSQUARE; }
"]" { return RSQUARE; }

"," { return COMMA; }
"." { return DOT; }
"->" { return ARROW; }

"+" { return ADD; }
"-" { return SUB; }
"*" { return MUL; }
"/" { return DIV; }
"%" { return REM; }

"==" { return EQ; }
"!=" { return NEQ; }

"<<" { return SHL; }
">>" { return SHR; }

">=" { return GTE; }
">" { return GT; }
"<=" { return LTE; }
"<" { return LT; }

"&" { return BITAND; }
"|" { return BITOR; }
"^" { return XOR; }

"&&" { return AND; }
"||" { return OR; }

"=" { return ASSIGN; }
"@" { return AT; }

. { report_unknown_character(yyextra->reports, node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(ctualloc, cturealloc, ctufree)
