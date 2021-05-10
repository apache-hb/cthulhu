%output "bison.c"
%defines "bison.h"

%define parse.error verbose // these messages are still awful but better than nothing
%define api.pure full
%lex-param { void *scanner }
%parse-param { void *scanner } { struct node_t **node }
%locations
%expect 0 // TODO: resolve dangling else without requiring compound stmts everywhere

%{
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <inttypes.h>
#include "ast.h"

int yylex();
int yyerror();

%}

%union {
    char *text;
    struct node_t *node;
}

%token<text>
    IDENT "identifier"
    DIGIT "integer literal"
    BDIGIT "binary integer literal"
    XDIGIT "hexadecimal integer literal"
    STRING "string literal"
    MSTRING "raw string literal"
    NIL "null"

%type<node>
    number string literal

%type<text>
    digit

%%

literal: number { $$ = loc($1, &@1); }
    | string { $$ = loc($1, &@1); }
    | NIL { $$ = loc(new_null(), &@1); }
    ;

number: digit { $$ = new_digit($1, NULL); }
    | digit IDENT { $$ = new_digit($1, $2); }
    ;

digit: DIGIT { $$ = $1; }
    | XDIGIT { $$ = $1; }
    | BDIGIT { $$ = $1; }
    ;

string: STRING { $$ = new_string($1); }
    | MSTRING { $$ = new_string($1); }
    ;

%%
