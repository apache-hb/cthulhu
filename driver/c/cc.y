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
    #include "sema.h"
}

%{
#include "scan.h"
int cclex();
void ccerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    ast_t *ast;

    char *ident;
    vector_t *vector;

    type_t *type;

    digit_t digit;
    sign_t sign;

    hlir_linkage_t storage;
    vardecl_t *vardecl;

    mpz_t mpz;
}

%token<ident>
    IDENT "identifier"
    TYPENAME "typename"

%token<mpz>
    DIGIT "integer literal"

%token
    /* module extensions */
    MODULE "_Module"
    IMPORT "_Import"

    EXTERN "extern"
    STATIC "static"
    AUTO "auto"

    SIGNED "signed"
    UNSIGNED "unsigned"

    SEMI ";"
    COMMA ","
    COLON2 "::"
    ASSIGN "="

    LBRACE "{"
    RBRACE "}"

%type<vector>
    path varlist

%type<storage>
    storage

%type<vardecl>
    var

%type<type>
    type

%type<sign>
    sign

%type<ast>
    expr

%start program

%%

program: modspec decls { cc_finish(x, @$); };

decls: decl | decls decl ;

decl: vardecl ;

vardecl: storage type varlist SEMI { cc_vardecl(x, $1, $2, $3); }
       ;

storage: %empty { $$ = LINK_EXPORTED; }
       | EXTERN { $$ = LINK_IMPORTED; }
       | STATIC { $$ = LINK_INTERNAL; }
       | AUTO { $$ = LINK_EXPORTED; }
       ;

varlist: var { $$ = vector_init($1); }
       | varlist COMMA var { vector_push(&$1, $3); $$ = $1; }
       ;

var: IDENT { $$ = new_vardecl(x, @$, $1, NULL); }
   | IDENT ASSIGN expr { $$ = new_vardecl(x, @$, $1, $3); }
   ;

/* TODO */
type: TYPENAME { $$ = NULL; }
    /* TODO: simplify this */
    | sign { $$ = type_digit(get_name_for_sign($1), node_new(x, @$), $1, DIGIT_INT); }
    ;

sign: SIGNED { $$ = SIGN_SIGNED; }
    | UNSIGNED { $$ = SIGN_UNSIGNED; }
    ;

/* TODO */
expr: DIGIT { $$ = ast_digit(x, @$, $1); } 
    ;

modspec: %empty
       | MODULE path SEMI { cc_module(x, $2); }
       ;

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

%%
