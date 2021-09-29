%option extra-type="scan_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="ctu"

%{
#include "ctu-bison.h"
#include "scan.h"
#include "ctu/util/report.h"
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

"var" { return VAR; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

[0-9]+ {
    mpz_init_set_str(yylval->digit, yytext, 10);
    return DIGIT;
}

";" { return SEMI; }

"::" { return COLON2; }
":" { return COLON; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }

"," { return COMMA; }

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

. { report(yyextra->reports, ERROR, node_new(yyextra, *yylloc), "unknown character %s", yytext); }

%%
