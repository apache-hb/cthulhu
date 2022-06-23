%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t x }
%locations
%expect 0
%define api.prefix {gen}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "gen/gen.h"
    #define YYLTYPE GENLTYPE
    #define YYSTYPE GENSTYPE
}

%{
int genlex();
void generror(where_t *where, void *state, scan_t scan, const char *msg);
%}

%union {
    ast_t *ast;
    map_t *map;
    vector_t *vector;
    pair_t *pair;

    char *str;
    mpz_t digit;
}

%token<str>
    IDENT "identifier"
    STRING "string literal"

%token<digit>
    DIGIT "digit literal"

%token
    CONFIG "config"
    TOKENS "tokens"
    GRAMMAR "grammar"
    TREE "tree"

    ASSIGN ":="
    RULE "::="

    LBRACE "{"
    RBRACE "}"

%type<ast>
    config
    expr

%type<map>
    map

%type<vector>
    fields

%type<pair>
    field

%start entry

%%

entry: config { scan_set(x, ast_root(x, @$, $1)); }
    ;

config: CONFIG map { $$ = ast_config(x, @$, $2); }
    ;

map: LBRACE fields RBRACE { $$ = collect_map(x, $2); }
    ;

fields: field { $$ = vector_init($1); }
    | fields field { vector_push(&$1, $2); $$ = $1; }
    ;

field: IDENT ASSIGN expr { $$ = pair_new($1, $3); }
    ;

expr: STRING { $$ = ast_string(x, @$, $1); }
    | IDENT { $$ = ast_string(x, @$, $1); }
    | DIGIT { $$ = ast_digit(x, @$, $1); }
    ;

%%
