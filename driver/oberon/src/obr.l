%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="obr"

%{
#include "obr-bison.h"
#include "oberon/scan.h"
#include "report/report-ext.h"
#include "interop/flex.h"
%}

%x COMMENT

%%

[ \r\n\t] ;
"(*" { BEGIN(COMMENT); }
<COMMENT>"*)" { BEGIN(INITIAL); }
<COMMENT>\n ;
<COMMENT>. ;

"+" { return PLUS; }
":=" { return ASSIGN; }
"-" { return MINUS; }
"^" { return CARET; }
"*" { return STAR; }
"=" { return EQUAL; }
"/" { return DIVIDE; }
"#" { return NEQUAL; }
"~" { return TILDE; }
">=" { return GTE; }
"<=" { return LTE; }
"<" { return LT; }
">" { return GT; }

"&" { return AND; }
"," { return COMMA; }
";" { return SEMI; }

".." { return DOT2; }
"." { return DOT; }

"|" { return BAR; }
":" { return COLON; }
"(" { return LPAREN; }
")" { return RPAREN; }
"[" { return LBRACKET; }
"]" { return RBRACKET; }

"{" { return LBRACE; }
"}" { return RBRACE; }

"ARRAY" { return ARRAY; }
"BEGIN" { return START; }
"BY" { return BY; }
"CASE" { return CASE; }
"CONST" { return CONST; }
"DIV" { return DIV; }
"DO" { return DO; }
"ELSE" { return ELSE; }
"ELSIF" { return ELSIF; }
"END" { return END; }
"EXIT" { return EXIT; }
"FOR" { return FOR; }
"IF" { return IF; }
"IMPORT" { return IMPORT; }
"IN" { return IN; }
"IS" { return IS; }
"LOOP" { return LOOP; }
"MOD" { return MOD; }
"MODULE" { return MODULE; }
"NIL" { return NIL; }
"OF" { return OF; }
"OR" { return OR; }
"POINTER" { return POINTER; }
"PROCEDURE" { return PROCEDURE; }
"RECORD" { return RECORD; }
"REPEAT" { return REPEAT; }
"RETURN" { return RETURN; }
"THEN" { return THEN; }
"TO" { return TO; }
"TYPE" { return TYPE; }
"UNTIL" { return UNTIL; }
"VAR" { return VAR; }
"WHILE" { return WHILE; }
"WITH" { return WITH; }

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval->ident = ctu_strdup(yytext);
    return IDENT;
}

[0-9]+ {
    mpz_init_set_str(yylval->number, yytext, 10);
    return NUMBER;
}

. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(obralloc, obrrealloc, obrfree)