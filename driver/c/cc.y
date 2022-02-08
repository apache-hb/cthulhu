%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cc}

%code requires {
    #define YYSTYPE CCSTYPE
    #define YYLTYPE CCLTYPE
    
    #include "scan.h"
}

%{
#include "scan.h"
int cclex();
void ccerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%start compilation_unit

%%

compilation_unit: %empty ;

%%
