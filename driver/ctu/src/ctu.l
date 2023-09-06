%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="ctu"

%{
#include "ctu-bison.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}

%x COMMENT

SUFFIX [a-zA-Z_]*

ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS [ \t\r\v\n\f]

%%

{WS} ;
"//".* ;
"/*" { BEGIN(COMMENT); }
<COMMENT>"*/" { BEGIN(INITIAL); }
<COMMENT>\n ;
<COMMENT>. ;

"->" { return ARROW; }

"+" { return PLUS; }
"-" { return MINUS; }
"*" { return STAR; }
"/" { return DIVIDE; }
"%" { return MODULO; }

"<<" { return SHL; }
">>" { return SHR; }

"<=" { return LTE; }
">=" { return GTE; }

"<" { return LT; }
">" { return GT; }

"==" { return EQ; }
"!=" { return NEQ; }

"&&" { return AND; }
"||" { return OR; }

"&" { return BITAND; }
"|" { return BITOR; }
"^" { return BITXOR; }

"!" { return NOT; }
"," { return COMMA; }

";" { return SEMI; }
":" { return COLON; }
"::" { return COLON2; }

"[" { return LSQUARE; }
"]" { return RSQUARE; }

"(" { return LPAREN; }
")" { return RPAREN; }

"{" { return LBRACE; }
"}" { return RBRACE; }

"=" { return ASSIGN; }

"module" { return MODULE; }
"import" { return IMPORT; }
"export" { return EXPORT; }

"def" { return DEF; }
"var" { return VAR; }
"const" { return CONST; }

"struct" { return STRUCT; }
"union" { return UNION; }
"type" { return TYPE; }

"return" { return RETURN; }
"while" { return WHILE; }
"if" { return IF; }
"else" { return ELSE; }

"break" { return BREAK; }
"continue" { return CONTINUE; }

"noinit" { return NOINIT; }

"as" { return AS; }

"$" { return DISCARD; }
"@" { return AT; }

"true" {
    yylval->boolean = true;
    return BOOLEAN;
}

"false" {
    yylval->boolean = false;
    return BOOLEAN;
}

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

"0b"[01]+ {
    yylval->digit = ctu_parse_digit(yyextra, yytext + 2, 2);
    return INTEGER;
}

"0x"[0-9a-fA-F]+ {
    yylval->digit = ctu_parse_digit(yyextra, yytext + 2, 16);
    return INTEGER;
}

[0-9]+ {
    yylval->digit = ctu_parse_digit(yyextra, yytext, 10);
    return INTEGER;
}

\"([^"\\\n]|{ESCAPES})*\" {
	yylval->string = ctu_parse_string(yyextra, yytext + 1, yyleng - 2);
    return STRING;
}

. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(ctualloc, cturealloc, ctufree)
