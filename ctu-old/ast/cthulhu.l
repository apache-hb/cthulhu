%option extra-type="scanner_t*"
%option yylineno 8bit nodefault
%option noyywrap noinput nounput
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="ct"

SUFFIX [u]?[tsilzpm]?

%{
#include "ctu-bison.h"
#include "ctu/ast/scanner.h"
#include "ctu/util/report.h"
#include "ctu/util/str.h"

#define YY_USER_ACTION flex_update(yylloc, yytext);

#define YY_INPUT(buffer, result, size) \
    result = flex_get(yyextra, buffer, size); \
    if (result <= 0) { result = YY_NULL; }

#define YY_USER_INIT flex_init(yylloc, 0)
%}

%%

[ \r\n\t] ;
"//".* ;

"def" { return DEF; }
"if" { return IF; }
"else" { return ELSE; }
"as" { return AS; }
"final" { return FINAL; }
"var" { return VAR; }
"while" { return WHILE; }
"break" { return BREAK; }
"continue" { return CONTINUE; }
"true" { return BOOL_TRUE; }
"false" { return BOOL_FALSE; }
"null" { return PTR_NULL; }
"return" { return RETURN; }
"export" { return EXPORTED; }
"struct" { return STRUCT; }
"union" { return UNION; }
"enum" { return ENUM; }
"import" { return IMPORT; }

"with" { return WITH; }
"of" { return OF; }

"$" { yylval->text = "$"; return IDENT; }
[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval->text = ctu_strdup(yytext); 
    return IDENT; 
}

[0-9]+{SUFFIX} { 
    yylval->digit.text = ctu_strdup(yytext); 
    yylval->digit.base = 10;
    return DIGIT; 
}

"0b"[01]+{SUFFIX} { 
    yylval->digit.text = ctu_strdup(yytext + 2); 
    yylval->digit.base = 2;
    return DIGIT; 
}

"0x"[0-9a-fA-F]+{SUFFIX} { 
    yylval->digit.text = ctu_strdup(yytext + 2);
    yylval->digit.base = 16;
    return DIGIT; 
}

\"(\\.|[^"\\])*\" { 
    yylval->text = ctu_strdup(yytext);
    return STRING;
}

"<<" { return SHL; }
">>" { return SHR; }
"^" { return XOR; }

"&&" { return AND; }
"||" { return OR; }

"." { return DOT; }
"->" { return ARROW; }

">=" { return GTE; }
"<=" { return LTE; }
">" { return GT; }
"<" { return LT; }
"==" { return EQ; }
"!=" { return NEQ; }

"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return REM; }

"&" { return BITAND; }
"|" { return BITOR; }

"(" { return LPAREN; }
")" { return RPAREN; }
"{" { return LBRACE; }
"}" { return RBRACE; }
"[" { return LSQUARE; }
"]" { return RSQUARE; }
"?" { return QUESTION; }
";" { return SEMI; }
"::" { return COLON2; }
":" { return COLON; }
"," { return COMMA; }
"=" { return ASSIGN; }
"@" { return AT; }

\0 { return END; }

. { report(LEVEL_ERROR, yyextra, *yylloc, "unknown character `%s`", yytext); }

%%