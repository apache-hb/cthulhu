%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cc}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "scan.h"
    #include "ast.h"
    
    #define YYSTYPE CCSTYPE
    #define YYLTYPE CCLTYPE
}

%{
int cclex();
void ccerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;

    struct {
        char *text;
        size_t length;
    } string;
}

%start program

%%

%%
