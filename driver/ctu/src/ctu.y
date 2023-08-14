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
    INTEGER "`integer literal`"

%token<boolean>
    BOOLEAN "`boolean literal`"

%token
    MODULE "`module`"
    IMPORT "`import`"
    EXPORT "`export`"

    DEF "`def`"
    VAR "`var`"
    CONST "`const`"

    STRUCT "`struct`"
    UNION "`union`"
    TYPE "`type`"

    NOINIT "`noinit`"

    AS "`as`"

    DISCARD "`$`"

    PLUS "`+`"
    MINUS "`-`"
    STAR "`*`"
    DIVIDE "`/`"
    MODULO "`%`"

    ASSIGN "`=`"
    SEMI "`;`"
    COLON "`:`"
    COLON2 "`::`"

    LBRACE "`{`"
    RBRACE "`}`"

%type<vector>
    path modspec
    imports importList
    decls declList
    structFields
    stmtList

%type<ast>
    import
    decl globalDecl functionDecl structDecl structField typeAliasDecl
    functionBody
    type
    expr maybeExpr
    stmt stmts localDecl

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
    | typeAliasDecl { $$ = $1; }
    ;

/* functions */

functionDecl: exported DEF IDENT COLON type functionBody { $$ = ctu_decl_function(x, @$, $1, $3, $5, $6); }
    ;

functionBody: ASSIGN expr SEMI { $$ = $2; }
    | stmts { $$ = $1; }
    ;

/* globals */

globalDecl: EXPORT mut IDENT COLON type ASSIGN maybeExpr SEMI { $$ = ctu_decl_global(x, @$, true, $2, $3, $5, $7); }
    | mut IDENT ASSIGN expr SEMI { $$ = ctu_decl_global(x, @$, false, $1, $2, NULL, $4); }
    | mut IDENT COLON type ASSIGN maybeExpr SEMI { $$ = ctu_decl_global(x, @$, false, $1, $2, $4, $6); }
    ;

exported: %empty { $$ = false; }
    | EXPORT { $$ = true; }
    ;

mut: CONST { $$ = false; }
    | VAR { $$ = true; }
    ;

/* type aliases */

typeAliasDecl: exported TYPE IDENT ASSIGN type SEMI { $$ = ctu_decl_typealias(x, @$, $1, $3, false, $5); }
    ;

/* structs */

structDecl: exported STRUCT IDENT LBRACE structFields RBRACE { $$ = ctu_decl_struct(x, @$, $1, $3, $5); }
    ;

structFields: structField { $$ = vector_init($1); }
    | structFields structField { vector_push(&$1, $2); $$ = $1; }
    ;

structField: ident COLON type SEMI { $$ = ctu_field(x, @$, $1, $3); }
    ;

/* types */

type: path { $$ = ctu_type_name(x, @$, $1); }
    | STAR type { $$ = ctu_type_pointer(x, @$, $2); }
    ;

/* statements */

stmtList: %empty { $$ = vector_of(0); }
    | stmtList stmt { vector_push(&$1, $2); $$ = $1; }
    ;

stmts: LBRACE stmtList RBRACE { $$ = ctu_stmt_list(x, @$, $2); }
    ;

localDecl: mut IDENT COLON type ASSIGN maybeExpr SEMI { $$ = ctu_stmt_local(x, @$, $1, $2, $4, $6); }
    | mut IDENT ASSIGN expr SEMI { $$ = ctu_stmt_local(x, @$, $1, $2, NULL, $4); }
    ;

stmt: expr SEMI { $$ = $1; }
    | stmts { $$ = $1; }
    | localDecl { $$ = $1; }
    ;

/* expressions */

maybeExpr: NOINIT { $$ = NULL; }
    | expr { $$ = $1; }
    ;

expr: INTEGER { $$ = ctu_expr_int(x, @$, $1.value); }
    | BOOLEAN { $$ = ctu_expr_bool(x, @$, $1); }
    | path { $$ = ctu_expr_name(x, @$, $1); }
    ;

/* basic */

ident: IDENT { $$ = $1; }
    | DISCARD { $$ = NULL; }
    ;

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

%%