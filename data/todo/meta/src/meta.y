%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {meta}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"
}

%code requires {
    #include "std/typed/vector.h"
    #include "std/vector.h"
    #include "std/map.h"
    #include "meta/ast.h"
    #include "meta/scan.h"
    #define YYSTYPE METASTYPE
    #define YYLTYPE METALTYPE
}

%{
int metalex(void *lval, void *loc, scan_t *scan);
void metaerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;
    bool boolean;
    meta_ast_t *ast;
    meta_field_t field;
    meta_config_t config;
    typevec_t *typevec;
    vector_t *vector;
    map_t *map;
}

%token<ident>
    TOK_IDENT "ident"
    TOK_STRING "string"

%token
    TOK_CONFIG "config"
    TOK_AST "ast"
    TOK_EXTENDS "extends"
    TOK_ABSTRACT "abstract"
    TOK_OPAQUE "opaque"
    TOK_VECTOR "vector"
    TOK_CSTRING "cstring"
    TOK_MPZ "mpz"

    TOK_ASSIGN "="
    TOK_COLON ":"
    TOK_SEMICOLON ";"
    TOK_LBRACE "{"
    TOK_RBRACE "}"
    TOK_LPAREN "("
    TOK_RPAREN ")"
    TOK_LBRACKET "["
    TOK_RBRACKET "]"

%type<ident>
    value extends

%type<boolean>
    abstract

%type<ast>
    type node

%type<field>
    field

%type<typevec>
    fields

%type<map> opt_config config

%type<config> option

%type<vector> nodes

%start program

%%

program: opt_config nodes { scan_set(x, meta_module(x, @$, $1, $2)); }
    ;

opt_config: %empty { $$ = NULL; }
    | config { $$ = $1; }
    ;

config: option { $$ = map_new(32, kTypeInfoString, scan_get_arena(x)); map_set($$, $1.name, (char*)$1.value); }
    | config option { map_set($1, $2.name, (char*)$2.value); $$ = $1; }
    ;

option: TOK_CONFIG TOK_IDENT TOK_ASSIGN value { $$ = meta_config_new($2, $4); }
    ;

value: TOK_IDENT
    | TOK_STRING
    ;

nodes: node { $$ = vector_init($1, scan_get_arena(x)); }
    | nodes node { vector_push(&$1, $2); $$ = $1;}
    ;

node: abstract TOK_AST TOK_IDENT extends TOK_LBRACE fields TOK_RBRACE { $$ = meta_node(x, @$, $3, $6, $4, $1); }
    ;

extends: %empty { $$ = NULL; }
    | TOK_EXTENDS TOK_IDENT { $$ = $2; }
    ;

abstract: %empty { $$ = false; }
    | TOK_ABSTRACT { $$ = true; }
    ;

fields: field { $$ = typevec_new(sizeof(meta_field_t), 4, scan_get_arena(x)); typevec_push($$, &$1); }
    | fields field { typevec_push($1, &$2); $$ = $1; }
    ;

field: TOK_IDENT TOK_COLON type { $$ = meta_field_new($1, $3); }
    ;

type: TOK_AST { $$ = meta_ast(x, @$); }
    | TOK_MPZ { $$ = meta_digit(x, @$); }
    | TOK_CSTRING { $$ = meta_string(x, @$); }
    | TOK_OPAQUE TOK_LPAREN TOK_IDENT TOK_RPAREN { $$ = meta_opaque(x, @$, $3); }
    | TOK_VECTOR TOK_LBRACKET type TOK_RBRACKET { $$ = meta_vector(x, @$, $3); }
    ;

%%
