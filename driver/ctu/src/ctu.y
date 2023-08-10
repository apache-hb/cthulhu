%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ctu}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "ctu/ast.h"
    #include "ctu/scan.h"
    #define YYSTYPE CTUSTYPE
    #define YYLTYPE CTULTYPE
}

%{
int ctulex(void *lval, void *loc, scan_t *scan);
void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;
    ctu_digit_t digit;

    bool boolean;

    vector_t *vector;

    ctu_t *ast;
}

%token<ident>
    IDENT "identifier"

%token<digit>
    INTEGER "integer"

%token<boolean>
    BOOLEAN "boolean"

%token
    MODULE "module"
    IMPORT "import"
    EXPORT "export"

    DEF "def"
    VAR "var"
    CONST "const"

    STRUCT "struct"
    UNION "union"

    NOINIT "noinit"

    AS "as"

    DISCARD "$"

    ASSIGN "="
    STAR "*"
    SEMI ";"
    COLON ":"
    COLON2 "::"

    LBRACE "{"
    RBRACE "}"

%type<vector>
    path modspec
    imports importList
    decls declList
    structFields structFieldList

%type<ast>
    import
    decl globalDecl functionDecl structDecl structField
    type
    expr maybeExpr

%type<ident>
    importAlias ident

%type<boolean>
    exported mut

%%

program: modspec imports decls { scan_set(x, ctu_module(x, @$, $1, $2, $3)); }
    ;

/* modules */

modspec: %empty { $$ = vector_of(0); }
    | MODULE path SEMI { $$ = $2; }
    ;

/* imports */

imports: %empty { $$ = vector_of(0); }
    | importList { $$ = $1; }
    ;

importList: import { $$ = vector_init($1); }
    | importList import { vector_push(&$1, $2); $$ = $1; }
    ;

import: IMPORT path importAlias SEMI { $$ = ctu_import(x, @$, $2, ($3 != NULL) ? $3 : vector_tail($2)); }
    ;

importAlias: %empty { $$ = NULL; }
    | AS IDENT { $$ = $2; }
    ;

/* toplevel decls */

decls: %empty { $$ = vector_of(0); }
    | declList { $$ = $1; }
    ;

declList: decl { $$ = vector_init($1); }
    | declList decl { vector_push(&$1, $2); $$ = $1; }
    ;

decl: globalDecl { $$ = $1; }
    | functionDecl { $$ = $1; }
    | structDecl { $$ = $1; }
    ;

/* functions */

functionDecl: exported DEF IDENT COLON type { $$ = ctu_decl_function(x, @$, $1, $3, $5); }
    ;

/* globals */

/* TODO: maybe globals should always need a type declared, at least exported ones */
globalDecl: exported mut IDENT COLON type ASSIGN maybeExpr SEMI { $$ = ctu_decl_global(x, @$, $1, $2, $3, $5, $7); }
    | exported mut IDENT ASSIGN expr SEMI { $$ = ctu_decl_global(x, @$, $1, $2, $3, NULL, $5); }
    ;

exported: %empty { $$ = false; }
    | EXPORT { $$ = true; }
    ;

mut: CONST { $$ = false; }
    | VAR { $$ = true; }
    ;

/* structs */

structDecl: exported STRUCT IDENT LBRACE structFields RBRACE { $$ = ctu_decl_struct(x, @$, $1, $3, $5); }
    ;

structFields: %empty { $$ = vector_of(0); }
    | structFieldList { $$ = $1; }
    ;

structFieldList: structField { $$ = vector_init($1); }
    | structFieldList structField { vector_push(&$1, $2); $$ = $1; }
    ;

structField: ident COLON type SEMI { $$ = ctu_field(x, @$, $1, $3); }
    ;

/* types */

type: path { $$ = ctu_type_name(x, @$, $1); }
    | STAR type { $$ = ctu_type_pointer(x, @$, $2); }
    ;

/* expressions */

maybeExpr: NOINIT { $$ = NULL; }
    | expr { $$ = $1; }
    ;

expr: INTEGER { $$ = ctu_expr_int(x, @$, $1.value); }
    | BOOLEAN { $$ = ctu_expr_bool(x, @$, $1); }
    ;

/* basic */

ident: IDENT { $$ = $1; }
    | DISCARD { $$ = NULL; }
    ;

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

%%