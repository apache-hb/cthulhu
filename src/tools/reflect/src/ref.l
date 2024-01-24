%option extra-type="scan_t*"
%option 8bit nodefault
%option noyywrap noinput nounput
%option noyyalloc noyyrealloc noyyfree
%option reentrant bison-bridge bison-locations
%option never-interactive batch
%option stack
%option prefix="ref"

%{
#include "ref_bison.h"
#include "interop/flex.h"
#include "interop/memory.h"
#include "cthulhu/util/text.h"
#include "cthulhu/events/events.h"
%}

%x COMMENT

%x ATTRIBUTE
%x TEMPLATE

WS [ \t\r\v\n\f]
ESCAPES (\\(['"\?\\abfnrtv]|[0-7]{1,3}|x[a-fA-F0-9]+))

%%

<INITIAL>"[[" { yy_push_state(ATTRIBUTE, yyscanner); return TOK_BEGIN_ATTRIBUTE; }
<ATTRIBUTE>"]]" { yy_pop_state(yyscanner); return TOK_END_ATTRIBUTE; }

<ATTRIBUTE>"alignas" { return TOK_ALIGNAS; }
<ATTRIBUTE>"config" { return TOK_CONFIG; }
<ATTRIBUTE>"brief" { return TOK_BRIEF; }
<ATTRIBUTE>"assert" { return TOK_ASSERT; }
<ATTRIBUTE>"remote" { return TOK_REMOTE; }

    /* reflection attributes */
<ATTRIBUTE>"transient" { return TOK_TRANSIENT; }
<ATTRIBUTE>"deprecated" { return TOK_DEPRECATED; }
<ATTRIBUTE>"typeid" { return TOK_TYPEID; }
<ATTRIBUTE>"cxxname" { return TOK_CXXNAME; }
<ATTRIBUTE>"facade" { return TOK_FACADE; }

    /* enum attributes */
<ATTRIBUTE>"bitflags" { return TOK_BITFLAGS; }
<ATTRIBUTE>"arithmatic" { return TOK_ARITHMATIC; }
<ATTRIBUTE>"iterator" { return TOK_ITERATOR; }
<ATTRIBUTE>"ordered" { return TOK_ORDERED; }

    /* type layout control */
<ATTRIBUTE>"layout" { return TOK_LAYOUT; }
<ATTRIBUTE>"packed" { return TOK_PACKED; }
<ATTRIBUTE>"system" { return TOK_SYSTEM; }
<ATTRIBUTE>"cbuffer" { return TOK_CBUFFER; }
<ATTRIBUTE>"internal" { return TOK_INTERNAL; }

<*>{WS}+ ;

<*>"//".* ;
<*>"/*" { yy_push_state(COMMENT, yyscanner); }
<COMMENT>"*/" { yy_pop_state(yyscanner); }
<COMMENT>.|\n ;

<*>"module" { return TOK_MODULE; }
<*>"import" { return TOK_IMPORT; }
<*>"api" { return TOK_API; }
<*>"final" { return TOK_SEALED; }

<*>"class" { return TOK_CLASS; }
<*>"virtual" { return TOK_VIRTUAL; }
<*>"variant" { return TOK_VARIANT; }
<*>"struct" { return TOK_STRUCT; }
<*>"using" { return TOK_ALIAS; }

<*>"public" { return TOK_PUBLIC; }
<*>"private" { return TOK_PRIVATE; }
<*>"protected" { return TOK_PROTECTED; }

<*>"export" { return TOK_EXPORT; }

<*>"def" { return TOK_DEF; }
<*>"const" { return TOK_CONST; }
<*>"case" { return TOK_CASE; }

<*>"new" { return TOK_NEW; }
<*>"delete" { return TOK_DELETE; }
<*>"default" { return TOK_DEFAULT; }

<*>"null" { return TOK_NULL; }
<*>"true" { return TOK_TRUE; }
<*>"false" { return TOK_FALSE; }
<*>"opaque" { return TOK_OPAQUE; }

<*>"in" { return TOK_IN; }
<*>"out" { return TOK_OUT; }
<*>"inout" { return TOK_INOUT; }

<*>"{" { return TOK_LBRACE; }
<*>"}" { return TOK_RBRACE; }

<*>"(" { return TOK_LPAREN; }
<*>")" { return TOK_RPAREN; }

<*>"[" { return TOK_LSQUARE; }
<*>"]" { return TOK_RSQUARE; }

<*>"&&" { return TOK_AND; }
<*>"||" { return TOK_OR; }

<*>"!<" { yy_push_state(TEMPLATE, yyscanner); return TOK_BEGIN_TEMPLATE; }
<TEMPLATE>">" { yy_pop_state(yyscanner); return TOK_END_TEMPLATE; }

<*>"<=" { return TOK_LTE; }
<*>"<" { return TOK_LT; }
<*>">=" { return TOK_GTE; }
<*>">" { return TOK_GT; }

<*>"<<" { return TOK_LSHIFT; }
<*>">>" { return TOK_RSHIFT; }

<*>"*" { return TOK_MUL; }
<*>"/" { return TOK_DIV; }
<*>"%" { return TOK_MOD; }
<*>"+" { return TOK_PLUS; }
<*>"-" { return TOK_MINUS; }
<*>"&" { return TOK_BITAND; }
<*>"|" { return TOK_BITOR; }
<*>"^" { return TOK_XOR; }

<*>"==" { return TOK_EQ; }
<*>"!=" { return TOK_NEQ; }
<*>"!" { return TOK_NOT; }

<*>"::" { return TOK_COLON2; }
<*>":" { return TOK_COLON; }
<*>";" { return TOK_SEMICOLON; }
<*>"." { return TOK_DOT; }
<*>"=" { return TOK_ASSIGN; }
<*>"," { return TOK_COMMA; }

<*>[a-zA-Z_][a-zA-Z0-9_]* {
    arena_t *arena = scan_get_arena(yyextra);
    yylval->ident = arena_strndup(yytext, yyleng, arena);
    return TOK_IDENT;
}

<*>"0b"[01]+ {
    ref_parse_digit(yylval->integer, yyextra, *yylloc, yytext + 2, 2);
    return TOK_INTEGER;
}

<*>"0o"[0-7]+ {
    ref_parse_digit(yylval->integer, yyextra, *yylloc, yytext + 2, 8);
    return TOK_INTEGER;
}

<*>"0x"[0-9a-fA-F]+ {
    ref_parse_digit(yylval->integer, yyextra, *yylloc, yytext + 2, 16);
    return TOK_INTEGER;
}

<*>[0-9]+ {
    ref_parse_digit(yylval->integer, yyextra, *yylloc, yytext, 10);
    return TOK_INTEGER;
}

<*>\"([^"\\\n]|{ESCAPES})*\" {
    ref_scan_t *scan = ref_scan_context(yyextra);
    arena_t *arena = scan_get_arena(yyextra);
	yylval->string = util_text_escape(scan->reports, node_new(yyextra, *yylloc), yytext + 1, yyleng - 2, arena);
    return TOK_STRING;
}

<*>. {
    ref_scan_t *scan = ref_scan_context(yyextra);
    evt_scan_unknown(scan->reports, node_new(yyextra, *yylloc), yytext);
}

%%

FLEX_MEMORY(ref)
