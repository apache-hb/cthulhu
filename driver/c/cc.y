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

    CHAR "char"
    SHORT "short"
    INT "int"
    LONG "long"

    BOOL "_Bool"
    VOID "void"

    SEMI ";"
    COMMA ","
    COLON2 "::"
    ASSIGN "="
    STAR "*"

    DOT3 "..."
    DOT "."

    LBRACE "{"
    RBRACE "}"

    LPAREN "("
    RPAREN ")"

%type<vector>
    path varlist

%type<storage>
    storage

%type<vardecl>
    var

%type<type>
    type elaborate_type opt_type

%type<sign>
    sign

%type<digit>
    inttype

%type<ast>
    expr

%start program

%%

program: modspec { cc_finish(x, @$); }
       | modspec decls { cc_finish(x, @$); }
       ;

decls: decl | decls decl ;

decl: vardecl ;

vardecl: storage opt_type varlist SEMI { cc_vardecl(x, $1, $3); }
       ;

storage: %empty { $$ = LINK_EXPORTED; }
       | EXTERN { $$ = LINK_IMPORTED; }
       | STATIC { $$ = LINK_INTERNAL; }
       | AUTO { $$ = LINK_EXPORTED; }
       ;

varlist: var { $$ = vector_init($1); }
       | varlist COMMA var { vector_push(&$1, $3); $$ = $1; }
       ;

var: elaborate_type IDENT { $$ = new_vardecl(x, @$, $1, $2, NULL); }
   | elaborate_type IDENT ASSIGN expr { $$ = new_vardecl(x, @$, $1, $2, $4); }
   ;

elaborate_type: %empty { $$ = get_current_type(x); }
              | STAR elaborate_type { $$ = type_pointer("pointer", get_current_type(x), node_new(x, @$)); }
              ;

opt_type: %empty { $$ = default_int(x, node_new(x, @$)); }
        | type { $$ = $1; }
        ;

type: TYPENAME { $$ = NULL; }
    | sign { $$ = get_digit($1, DIGIT_INT); set_current_type(x, $$); }
    | inttype { $$ = get_digit(SIGN_DEFAULT, $1); set_current_type(x, $$); }
    | sign inttype { $$ = get_digit($1, $2); set_current_type(x, $$); }
    | VOID { $$ = get_void(); set_current_type(x, $$); }
    | BOOL { $$ = get_bool(); set_current_type(x, $$); }
    ;

inttype: CHAR { $$ = DIGIT_CHAR; }
       | SHORT { $$ = DIGIT_SHORT; }
       | INT { $$ = DIGIT_INT; }
       | LONG { $$ = DIGIT_LONG; }
       | SHORT INT { $$ = DIGIT_SHORT; }
       | LONG INT { $$ = DIGIT_LONG; }
       | LONG LONG { $$ = DIGIT_LONG; }
       | LONG LONG INT { $$ = DIGIT_LONG; }
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
