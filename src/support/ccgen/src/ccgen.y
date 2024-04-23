%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ccgen}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"
}

%code requires {
    #include "ccgen/ast.h"
    #include "ccgen/scan.h"
    #define YYSTYPE STYPE
    #define YYLTYPE LTYPE
}

%{
int ccgenlex(void *lval, void *loc, scan_t *scan);
void ccgenerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%token
    LT "<"
    GT ">"

    COLON ":"
    SEMI ";"
    BAR "|"

%token
    ID "identifier"

    RULE "rule"
    PATTERN "pattern"

%start program

%%

program: pattern_list rule_list
    ;

/* lexing patterns */

pattern_list: pattern
    | pattern_list pattern
    ;

pattern: PATTERN ID COLON pattern_body SEMI
    | PATTERN LT ID GT ID COLON pattern_body SEMI
    ;

pattern_body: ID
    ;

/* parsing rules */

rule_list: rule
    | rule_list rule
    ;

rule: RULE ID COLON rule_body SEMI
    | RULE LT ID GT ID COLON rule_body SEMI
    ;

rule_body: rule_body BAR rule_body
    | ID
    ;

%%
