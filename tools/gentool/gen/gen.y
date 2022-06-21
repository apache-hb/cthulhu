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
    char *ident;
}

%token<ident>
    IDENT "identifier"

%token
    CONFIG "config"
    TOKENS "tokens"
    GRAMMAR "grammar"
    TREE "tree"

    ASSIGN ":="

    LBRACE "{"
    RBRACE "}"

%start entry

%%

entry: config tokens grammar ast ;

config: CONFIG map ;

tokens: TOKENS map ;

grammar: GRAMMAR map ;

ast : TREE IDENT map ;

map: LBRACE fields RBRACE ;

fields: field | fields field ;

field: IDENT ASSIGN IDENT ;

%%
