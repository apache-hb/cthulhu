%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="ctu"

%{
#include "ctu_bison.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "cthulhu/util/text.h"
#include "cthulhu/broker/scan.h"
%}

%x COMMENT

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
"..." { return DOT3; }
"." { return DOT; }

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
"variant" { return VARIANT; }

"return" { return RETURN; }
"while" { return WHILE; }
"for" { return FOR; }
"if" { return IF; }
"else" { return ELSE; }

"break" { return BREAK; }
"continue" { return CONTINUE; }

"default" { return DEFAULT; }
"case" { return CASE; }

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
    arena_t *arena = ctx_get_string_arena(yyextra);
    yylval->ident = arena_strndup(yytext, yyleng, arena);
    return IDENT;
}

"0b"[01]+ {
    ctu_parse_digit(yyextra, *yylloc, yylval->digit, yytext + 2, 2);
    return INTEGER;
}

"0x"[0-9a-fA-F]+ {
    ctu_parse_digit(yyextra, *yylloc, yylval->digit, yytext + 2, 16);
    return INTEGER;
}

[0-9]+ {
    ctu_parse_digit(yyextra, *yylloc, yylval->digit, yytext, 10);
    return INTEGER;
}

\"([^"\\\n]|{ESCAPES})*\" {
    arena_t *arena = ctx_get_string_arena(yyextra);
    logger_t *logger = ctx_get_logger(yyextra);
	yylval->string = util_text_escape(logger, node_new(yyextra, *yylloc), yytext + 1, yyleng - 2, arena);
    return STRING;
}

. { ctx_unknown_symbol(yyextra, yylloc, yytext); }

%%

FLEX_MEMORY(ctu)
