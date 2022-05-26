%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {gen}

%code top {
    #include "cthulhu/ast/interop.h"
}

%code requires {
    #include "ast.h"
    
    #define YYSTYPE GENSTYPE
    #define YYLTYPE GENLTYPE
}

%{
int genlex();
void generror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *text;
    vector_t *vector;
    map_t *map;
    ast_t *ast;
}


%token<text>
    STRING "string literal"
    IDENT "identifier"
    CODE "code block"

%token
    CONFIG "config"
    LEXER "lexer"
    PARSER "parser"

    CODE_BEGIN "{{"
    CODE_END "}}"

    LBRACE "{"
    RBRACE "}"
    LSQUARE "["
    RSQUARE "]"
    LPAREN "("
    RPAREN ")"

    COMMA ","
    ASSIGN ":="
    COLON2 "::"

    RULE "::="
    PLUS "+"
    STAR "*"
    QUESTION "?"
    PIPE "|"

%type<ast>
    value configItem sequence rule

%type<vector>
    vector optVectorBody vectorBody configBody
    sequenceBody rules body

%type<map>
    config lexer parser

%start grammar

%%

grammar: config lexer parser rules { scan_set(x, ast_grammar(x, @$, $1, $2, $3, $4)); }
    ;

rules: rule { $$ = vector_init($1); }
    | rules rule { vector_push(&$1, $2); $$ = $1; }
    ;

rule: IDENT RULE body CODE { $$ = ast_rule(x, @$, $1, $3); }
    ;

body: IDENT { $$ = vector_init($1); }
    | body IDENT { vector_push(&$1, $2); $$ = $1; }
    ;

parser: PARSER LBRACE configBody RBRACE { $$ = build_map(x, @$, $3); }
    ;

lexer: LEXER LBRACE configBody RBRACE { $$ = build_map(x, @$, $3); }
    ;

config: CONFIG LBRACE configBody RBRACE { $$ = build_map(x, @$, $3); }
    ;

configBody: configItem { $$ = vector_init($1); }
    | configBody configItem { vector_push(&$1, $2); $$ = $1; }
    ;

configItem: IDENT ASSIGN value { $$ = ast_pair(x, @$, $1, $3); }
    ;

value: STRING { $$ = ast_string(x, @$, $1); }
    | IDENT { $$ = ast_ident(x, @$, $1); }
    | CODE { $$ = ast_string(x, @$, $1); }
    | vector { $$ = ast_vector(x, @$, $1); }
    | sequence { $$ = $1; }
    ;

sequence: LPAREN sequenceBody RPAREN { $$ = ast_vector(x, @$, $2); }
    ; 

sequenceBody: value { $$ = vector_init($1); }
    | sequenceBody COLON2 value { vector_push(&$1, $3); $$ = $1; }
    ;

vector: LSQUARE optVectorBody RSQUARE { $$ = $2; }
    ;

optVectorBody: %empty { $$ = vector_new(0); }
    | vectorBody { $$ = $1; }
    ;

vectorBody: value { $$ = vector_init($1); }
    | vectorBody value { vector_push(&$1, $2); $$ = $1; }
    ;

%%
