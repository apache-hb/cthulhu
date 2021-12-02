%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {bf}

%code requires {
    #define YYSTYPE BFSTYPE
    #define YYLTYPE BFLTYPE
    
    #include "scan.h"
}

%{
#include "scan.h"
int bflex();
void bferror(where_t *where, void *state, scan_t *scan, const char *msg);    
%}

%token
    ADD "+"
    SUB "-"
    INC ">"
    DEC "<"
    OUT "."
    IN ","
    LBRACKET "["
    RBRACKET "]"

%start start

%%
start: inst
    | inst start
    ;

inst: ADD { printf("add"); }
    | SUB { printf("sub"); }
    | INC { printf("inc"); }
    | DEC { printf("dec"); }
    | OUT { printf("out"); }
    | IN { printf("in"); }
    | LBRACKET { printf("["); }
    | RBRACKET { printf("]"); }
    ;
%%
