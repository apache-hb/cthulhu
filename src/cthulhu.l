%option outfile="flex.c" header-file="flex.h"

%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations

%{
#include <stdio.h>
#include "bison.h"

#define YY_EXTRA_TYPE scan_extra_t *
#define YY_USER_ACTION                                               \
    yylloc->first_line = yylloc->last_line;                          \
    yylloc->first_column = yylloc->last_column;                      \
    if (yylloc->last_line == yylineno)                               \
      yylloc->last_column += yyleng;                                 \
    else {                                                           \
      yylloc->last_line = yylineno;                                  \
      yylloc->last_column = yytext + yyleng - strrchr(yytext, '\n'); \
    }

%}

%%

[ \r\n\t] ;
"//".* ;
[0-9]+ { yylval->text = strdup(yytext); return DIGIT; }
"0x"[0-9a-fA-F]+ { yylval->text = strdup(yytext); return XDIGIT; }
"0b"[01]+ { yylval->text = strdup(yytext); return BDIGIT; }
\"\"\"(\\.|[^"\\])*\"\"\" { yylval->text = strdup(yytext); return MSTRING; }
\"(\\.|[^"\\])*\" { yylval->text = strdup(yytext); return STRING; }

"def" { return DEF; }
"as" { return AS; }
"var" { return VAR; }
"final" { return FINAL; }
"export" { return EXPORT; }
"import" { return IMPORT; }
"module" { return MODULE; }
"record" { return RECORD; }
"union" { return UNION; }
"variant" { return VARIANT; }
"using" { return USING; }
"return" { return RETURN; }
"raise" { return RAISE; }
"continue" { return CONTINUE; }
"compile" { return COMPILE; }
"asm" { return ASM; }
"break" { return BREAK; }
"while" { return WHILE; }
"for" { return FOR; }
"if" { return IF; }
"else" { return ELSE; }
"with" { return WITH; } 
"then" { return THEN; }
"null" { return NIL; }

[a-zA-Z_][a-zA-Z0-9_]* { yylval->text = strdup(yytext); return IDENT; }
"$" { yylval->text = strdup(yytext); return IDENT; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }
"[" { return LSQUARE; }
"]" { return RSQUARE; }

"+=" { return ADDEQ; }
"-=" { return SUBEQ; }
"/=" { return DIVEQ; }
"*=" { return MULEQ; }
"%=" { return REMEQ; }
"&=" { return ANDEQ; }
"|=" { return OREQ; }
"^=" { return XOREQ; }
"<<=" { return SHLEQ; }
">>=" { return SHREQ; }

"==" { return EQ; }
"!=" { return NEQ; }
"<<" { return SHL; }
">>" { return SHR; }
">=" { return GTE; }
">" { return GT; }
"<=" { return LTE; }
"<" { return LT; }
"&&" { return AND; }
"||" { return OR; }
"&" { return BITAND; }
"|" { return BITOR; }
"^" { return BITXOR; }
"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return REM; }
"!" { return NOT; }

"," { return COMMA; }
"=" { return ASSIGN; }
"?" { return QUESTION; }
"::" { return COLON2; }
":" { return COLON; }
".." { return DOT2; }
"." { return DOT; }
"->" { return ARROW; }
";" { return SEMI; }
"@" { return AT; }

. { fprintf(stderr, "unknown characters `%s`\n", yytext); }

%%
