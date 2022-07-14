%option extra-type="scan_t"
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

"~" { return BITNOT; }

"->" { return ARROW; }

"!<" { 
    enter_template(yyextra);
    return BEGIN_TEMPLATE;
}

"..." { return DOT3; }
"@" { return AT; }

"::" { return COLON2; }
":" { return COLON; }
";" { return SEMICOLON; }

">>" { return RSHIFT; }
"<<" { return LSHIFT; }

">=" { return GTE; }
"<=" { return LTE; }

"<" { return LT; }
">" { 
    if (exit_template(yyextra) > 0) {
        return END_TEMPLATE;
    } else {
        return GT; 
    }
}

"==" { return EQ; }
"!=" { return NEQ; }

"!" { return NOT; }
"=" { return EQUALS; }

"." { return DOT; }
"," { return COMMA; }

"+" { return ADD; }
"-" { return SUB; }
"/" { return DIV; }
"*" { return MUL; }
"%" { return MOD; }

"&&" { return AND; }
"||" { return OR; }

"&" { return BITAND; }
"|" { return BITOR; }
"^" { return BITXOR; }

"(" { return LPAREN; }
")" { return RPAREN; }

"{" { return LBRACE; }
"}" { return RBRACE; }

"[" { return LSQUARE; }
"]" { return RSQUARE; }

"module" { return MODULE; }
"import" { return IMPORT; }
"export" { return EXPORT; }
"using" { return USING; }

"def" { return DEF; }
"var" { return VAR; }
"final" { return FINAL; }
"const" { return CONST; }

"object" { return OBJECT; }
"struct" { return STRUCT; }
"union" { return UNION; }
"variant" { return VARIANT; }
"type" { return TYPE; }

"if" { return IF; }
"else" { return ELSE; }
"while" { return WHILE; }
"for" { return FOR; }
"break" { return BREAK; }
"continue" { return CONTINUE; }
"return" { return RETURN; }

"lambda" { return LAMBDA; }
"as" { return AS; }

"__null" { return NIL; }
"noinit" { return NOINIT; }

"__match" { return MATCH; }
"__default" { return DEFAULT; }
"__case" { return CASE; }
"__select" { return SELECT; }
"__order" { return ORDER; }
"__on" { return ON; }
"__from" { return FROM; }
"__where" { return WHERE; }
"__when" { return WHEN; }
"__then" { return THEN; }
"__with" { return WITH; }

"__in" { return IN; }
"__out" { return OUT; }
"__inout" { return INOUT; }

"__private" { return PRIVATE; }
"__public" { return PUBLIC; }
"__protected" { return PROTECTED; }

"__async" { return ASYNC; }
"__await" { return AWAIT; }
"__yield" { return YIELD; }

"__unique" { return UNIQUE; }
"__shared" { return SHARED; }
"__strong" { return STRONG; }
"__weak" { return WEAK; }
"__move" { return MOVE; }

"__compile" { return COMPILED; }
"__static" { return COMPILED; }
"__explicit" { return EXPLICIT; }
"__inline" { return INLINE; }

"__try" { return TRY; }
"__catch" { return CATCH; }
"__finally" { return FINALLY; }
"__throw" { return THROW; }
"__raise" { return THROW; }
"__except" { return THROW; }

"__contract" { return CONTRACT; }
"__requires" { return REQUIRES; }
"__assert" { return ASSERT; }
"__ensures" { return ENSURES; }
"__invariant" { return INVARIANT; }

"__sizeof" { return SIZEOF; }
"__alignof" { return ALIGNOF; }
"__offsetof" { return OFFSETOF; }
"__typeof" { return TYPEOF; }
"__uuidof" { return UUIDOF; }

"true" { 
    yylval->boolean = true;
    return BOOLEAN;
}

"false" { 
    yylval->boolean = false;
    return BOOLEAN;
}

"$" { 
    yylval->ident = NULL;
    return IDENT; 
}

[a-zA-Z_][a-zA-Z0-9_]* { 
    yylval->ident = ctu_strdup(yytext);
    return IDENT; 
}

"0b"[01]+{SUFFIX} {
    yylval->digit.suffix = init_string_with_suffix(yylval->digit.mpz, yytext + 2, 2);
    return INTEGER;
}

"0x"[0-9a-fA-F]+{SUFFIX} {
    yylval->digit.suffix = init_string_with_suffix(yylval->digit.mpz, yytext + 2, 16);
    return INTEGER;
}

[0-9]+{SUFFIX} {
    yylval->digit.suffix = init_string_with_suffix(yylval->digit.mpz, yytext, 10);
    return INTEGER;
}

\"([^"\\\n]|{ESCAPES})*\" {
	yylval->string.data = ctu_strndup(yytext + 1, yyleng - 2);
    yylval->string.length = yyleng - 2;
    return STRING;
}

. { report_unknown_character(scan_reports(yyextra), node_new(yyextra, *yylloc), yytext); }

%%

FLEX_MEMORY(ctualloc, cturealloc, ctufree)
