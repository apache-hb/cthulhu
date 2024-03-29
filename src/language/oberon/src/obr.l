%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option prefix="obr"

%{
#include "obr_bison.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "arena/arena.h"
#include "scan/node.h"
#include "cthulhu/util/text.h"
#include "cthulhu/broker/scan.h"
%}

%x COMMENT

ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))
WS [ \t\r\v\n\f]

%%

{WS} ;
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
    arena_t *arena = ctx_get_string_arena(yyextra);
    yylval->ident = arena_strdup(yytext, arena);
    return IDENT;
}

[0-9]+ {
    mpz_init_set_str(yylval->number, yytext, 10);
    return NUMBER;
}

\"([^"\\\n]|{ESCAPES})*\" {
    arena_t *arena = ctx_get_string_arena(yyextra);
    logger_t *logger = ctx_get_logger(yyextra);
	yylval->string = util_text_escape(logger, node_new(yyextra, *yylloc), yytext + 1, yyleng - 2, arena);
    return STRING;
}

. { ctx_unknown_symbol(yyextra, yylloc, yytext); }

%%

FLEX_MEMORY(obr)
