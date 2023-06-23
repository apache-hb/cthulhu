%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="lox"

%{
#include "lox-bison.h"
#include "scan.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}

ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS [ \t\r\v\n\f]

%%

{WS} ;
"//".* ;

"and" { return AND; }
"or" { return OR; }

"true" { return TRUE; }
"false" { return FALSE; }
"nil" { return NIL; }

"class" { return CLASS; }
"super" { return SUPER; }
"this" { return THIS; }

"if" { return IF; }
"else" { return ELSE; }
"for" { return FOR; }
"while" { return WHILE; }
"return" { return RETURN; }

"fun" { return FUN; }
"var" { return VAR; }

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval->ident = ctu_strdup(yytext);
    return IDENT; 
}

[0-9]+ { 
    return DIGIT; 
}

\"([^"\\\n]|{ESCAPES})*\" {
    return STRING;
}


. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(loxalloc, loxrealloc, loxfree)
