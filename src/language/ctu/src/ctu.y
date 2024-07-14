%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ctu}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"
    #include "cthulhu/broker/scan.h"
    #include "std/vector.h"
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
    text_t string;
    ctu_integer_t digit;
    bool boolean;

    vector_t *vector;
    const vector_t *cvector;

    ctu_t *ast;

    ctu_params_t params;
}

%token<ident>
    IDENT "identifier"

%token<digit>
    INTEGER "`integer literal`"

%token<boolean>
    BOOLEAN "`boolean literal`"

%token<string>
    STRING "`string literal`"

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
    VARIANT "`variant`"

    RETURN "`return`"
    WHILE "`while`"
    FOR "`for`"
    IF "`if`"
    ELSE "`else`"

    BREAK "`break`"
    CONTINUE "`continue`"

    NOINIT "`noinit`"

    DEFAULT "`default`"
    CASE "`case`"

    AS "`as`"

    DISCARD "`$`"
    AT "`@`"
    ARROW "`->`"

    PLUS "`+`"
    MINUS "`-`"
    STAR "`*`"
    DIVIDE "`/`"
    MODULO "`%`"

    NOT "`!`"
    DOT "`.`"
    DOT3 "`...`"

    SHL "`<<`"
    SHR "`>>`"
    BITAND "`&`"
    BITOR "`|`"
    BITXOR "`^`"

    LT "`<`"
    GT "`>`"
    LTE "`<=`"
    GTE "`>=`"

    EQ "`==`"
    NEQ "`!=`"

    AND "`&&`"
    OR "`||`"

    COMMA "`,`"
    ASSIGN "`=`"
    SEMI "`;`"
    COLON "`:`"
    COLON2 "`::`"

    LSQUARE "`[`"
    RSQUARE "`]`"

    LPAREN "`(`"
    RPAREN "`)`"

    LBRACE "`{`"
    RBRACE "`}`"

%type<vector>
    path
    import_seq
    decl_seq
    record_fields
    stmt_list
    fn_param_list
    attrib_args
    expr_list
    type_list
    variant_field_list
    init_list opt_init_list
    attribs attrib attrib_body attrib_list

%type<cvector>
    modspec imports opt_type_list opt_expr_list decls opt_variant_field_list

/**
 * order of operations, tightest first
 * === all mathematical operations come first ===
 * 1: unary ops `!expr` `-expr` `+expr`
 * 2: multiplication `lhs * rhs` `lhs / rhs` `lhs % rhs`
 * 3: addition `lhs + rhs` `lhs - rhs`
 * === then bitwise operations ===
 * 4: bitwise shift `lhs << rhs` `lhs >> rhs`
 * 5: bitwise ops `lhs & rhs` `lhs | rhs`
 * 6: bitwise xor `lhs ^ rhs`
 * === then comparisons ===
 * 7: comparisons `lhs < rhs` `lhs <= rhs` `lhs > rhs` `lhs >= rhs`
 * 8: equality `lhs == rhs` `lhs != rhs`
 * === then boolean operations ===
 * 9: boolean and `lhs && rhs`
 * 10: boolean or `lhs || rhs`
 */

%type<ast>
    import
    decl inner_decl global_decl fn_decl struct_decl union_decl record_field type_alias_decl
    fn_body
    type
    primary_expr expr
    or_expr and_expr eq_expr compare_expr xor_expr bit_expr shift_expr add_expr mul_expr unary_expr postfix_expr
    opt_expr
    stmt stmts local_decl return_stmt while_stmt assign_stmt branch_stmt
    fn_param fn_result
    variant_decl variant_field underlying
    field_init init
    single_attrib

%type<ident>
    import_alias ident opt_ident while_name variadic

%type<boolean>
    exported mut is_default

%type<params>
    fn_params

%%

program: modspec imports decls { scan_set(x, ctu_module(x, @$, $1, $2, $3)); }
    ;

/* modules */

modspec: %empty { $$ = &kEmptyVector; }
    | MODULE path SEMI { $$ = $2; }
    ;

/* imports */

imports: %empty { $$ = &kEmptyVector; }
    | import_seq { $$ = $1; }
    ;

import_seq: import { $$ = ctx_vector_init($1, x); }
    | import_seq import { vector_push(&$1, $2); $$ = $1; }
    ;

import: IMPORT path import_alias SEMI { $$ = ctu_import(x, @$, $2, ($3 != NULL) ? $3 : vector_tail($2)); }
    ;

import_alias: %empty { $$ = NULL; }
    | AS IDENT { $$ = $2; }
    ;

/* decorators */

attribs: %empty { $$ = ctx_vector_new(4, x); }
    | attribs attrib { vector_append(&$1, $2); $$ = $1; }
    ;

attrib: AT attrib_body { $$ = $2; }
    ;

attrib_body: single_attrib { $$ = ctx_vector_init($1, x); }
    | LSQUARE attrib_list RSQUARE { $$ = $2; }
    ;

attrib_list: single_attrib { $$ = ctx_vector_init($1, x); }
    | attrib_list COMMA single_attrib { vector_push(&$1, $3); $$ = $1; }
    ;

single_attrib: path { $$ = ctu_attrib(x, @$, $1, &kEmptyVector); }
    | path LPAREN RPAREN { $$ = ctu_attrib(x, @$, $1, &kEmptyVector); }
    | path LPAREN attrib_args RPAREN { $$ = ctu_attrib(x, @$, $1, $3); }
    ;

attrib_args: expr { $$ = ctx_vector_init($1, x); }
    | attrib_args COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

/* toplevel decls */

decls: %empty { $$ = &kEmptyVector; }
    | decl_seq { $$ = $1; }
    ;

decl_seq: decl { $$ = ctx_vector_init($1, x); }
    | decl_seq decl { vector_push(&$1, $2); $$ = $1; }
    ;

decl: attribs inner_decl { $$ = ctu_apply($2, $1); }
    ;

inner_decl: global_decl { $$ = $1; }
    | fn_decl { $$ = $1; }
    | struct_decl { $$ = $1; }
    | type_alias_decl { $$ = $1; }
    | variant_decl { $$ = $1; }
    | union_decl { $$ = $1; }
    ;

/* variants/enums */

variant_decl: exported VARIANT IDENT underlying LBRACE opt_variant_field_list RBRACE { $$ = ctu_decl_variant(x, @$, $1, $3, $4, $6); }
    ;

underlying: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

opt_variant_field_list: %empty { $$ = &kEmptyVector; }
    | variant_field_list { $$ = $1; }
    ;

variant_field_list: variant_field { $$ = ctx_vector_init($1, x); }
    | variant_field_list variant_field { vector_push(&$1, $2); $$ = $1; }
    ;

variant_field: is_default ident ASSIGN expr { $$ = ctu_variant_case(x, @$, $2, $1, $4); }
    ;

is_default: DEFAULT { $$ = true; }
    | CASE { $$ = false; }
    ;

/* functions */

fn_decl: exported DEF IDENT fn_params fn_result fn_body { $$ = ctu_decl_function(x, @$, $1, $3, $4.params, $4.variadic, $5, $6); }
    ;

fn_result: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

fn_params: %empty { $$ = ctu_params_new(&kEmptyVector, NULL); }
    | LPAREN fn_param_list variadic RPAREN { $$ = ctu_params_new($2, $3); }
    ;

variadic: %empty { $$ = NULL; }
    | COMMA IDENT COLON DOT3 { $$ = $2; }
    ;

fn_param_list: fn_param { $$ = ctx_vector_init($1, x); }
    | fn_param_list COMMA fn_param { vector_push(&$1, $3); $$ = $1; }
    ;

fn_param: IDENT COLON type { $$ = ctu_param(x, @$, $1, $3); }
    ;

fn_body: ASSIGN expr SEMI { $$ = ctu_stmt_return(x, @$, $2); }
    | stmts { $$ = $1; }
    | SEMI { $$ = NULL; }
    ;

/* globals */

global_decl: EXPORT mut IDENT COLON type ASSIGN opt_expr SEMI { $$ = ctu_decl_global(x, @$, true, $2, $3, $5, $7); }
    | mut IDENT ASSIGN expr SEMI { $$ = ctu_decl_global(x, @$, false, $1, $2, NULL, $4); }
    | mut IDENT COLON type ASSIGN opt_expr SEMI { $$ = ctu_decl_global(x, @$, false, $1, $2, $4, $6); }
    ;

exported: %empty { $$ = false; }
    | EXPORT { $$ = true; }
    ;

mut: CONST { $$ = false; }
    | VAR { $$ = true; }
    ;

/* type aliases */

type_alias_decl: exported TYPE IDENT ASSIGN type SEMI { $$ = ctu_decl_typealias(x, @$, $1, $3, false, $5); }
    ;

/* structs */

struct_decl: exported STRUCT IDENT LBRACE record_fields RBRACE { $$ = ctu_decl_struct(x, @$, $1, $3, $5); }
    ;

/* unions */

union_decl: exported UNION IDENT LBRACE record_fields RBRACE { $$ = ctu_decl_union(x, @$, $1, $3, $5); }
    ;

/* fields */

record_fields: record_field { $$ = ctx_vector_init($1, x); }
    | record_fields record_field { vector_push(&$1, $2); $$ = $1; }
    ;

record_field: ident COLON type SEMI { $$ = ctu_field(x, @$, $1, $3); }
    ;

/* types */

type: path { $$ = ctu_type_name(x, @$, $1); }
    | STAR type { $$ = ctu_type_pointer(x, @$, $2); }
    | LSQUARE STAR RSQUARE type { $$ = ctu_type_pointer(x, @$, $4); /* TODO: implement indexable pointers */ }
    | DEF LPAREN opt_type_list RPAREN ARROW type { $$ = ctu_type_function(x, @$, $3, $6); }
    | LSQUARE expr RSQUARE type { $$ = ctu_type_array(x, @$, $4, $2); }
    ;

type_list: type { $$ = ctx_vector_init($1, x); }
    | type_list COMMA type { vector_push(&$1, $3); $$ = $1; }
    ;

opt_type_list: %empty { $$ = &kEmptyVector; }
    | type_list { $$ = $1; }
    ;

/* statements */

stmt_list: %empty { $$ = ctx_vector_of(0, x); }
    | stmt_list stmt { vector_push(&$1, $2); $$ = $1; }
    ;

while_name: %empty { $$ = NULL; }
    | COLON2 IDENT { $$ = $2; }
    ;

while_stmt: WHILE while_name expr stmts { $$ = ctu_stmt_while(x, @$, $2, $3, $4, NULL); }
    | WHILE while_name expr stmts ELSE stmts { $$ = ctu_stmt_while(x, @$, $2, $3, $4, $6); }
    ;

return_stmt: RETURN expr SEMI { $$ = ctu_stmt_return(x, @$, $2); }
    | RETURN SEMI { $$ = ctu_stmt_return(x, @$, NULL); }
    ;

stmts: LBRACE stmt_list RBRACE { $$ = ctu_stmt_list(x, @$, $2); }
    ;

local_decl: mut IDENT COLON type ASSIGN opt_expr SEMI { $$ = ctu_stmt_local(x, @$, $1, $2, $4, $6); }
    | mut IDENT ASSIGN expr SEMI { $$ = ctu_stmt_local(x, @$, $1, $2, NULL, $4); }
    ;

assign_stmt: expr ASSIGN expr SEMI { $$ = ctu_stmt_assign(x, @$, $1, $3); }
    ;

branch_stmt: IF expr stmts { $$ = ctu_stmt_branch(x, @$, $2, $3, NULL); }
    | IF expr stmts ELSE stmts { $$ = ctu_stmt_branch(x, @$, $2, $3, $5); }
    ;

stmt: expr SEMI { $$ = $1; }
    | stmts { $$ = $1; }
    | return_stmt { $$ = $1; }
    | local_decl { $$ = $1; }
    | while_stmt { $$ = $1; }
    | assign_stmt { $$ = $1; }
    | branch_stmt { $$ = $1; }
    | BREAK opt_ident SEMI { $$ = ctu_stmt_break(x, @$, $2); }
    | CONTINUE opt_ident SEMI { $$ = ctu_stmt_continue(x, @$, $2); }
    ;

/* init */

init: DOT LBRACE opt_init_list RBRACE { $$ = ctu_expr_init(x, @$, $3); }
    ;

opt_init_list: %empty { $$ = ctx_vector_of(0, x); }
    | init_list { $$ = $1; }
    ;

init_list: field_init { $$ = ctx_vector_init($1, x); }
    | init_list COMMA field_init { vector_push(&$1, $3); $$ = $1; }
    ;

field_init: ident ASSIGN expr { $$ = ctu_field_init(x, @$, $1, $3); }
    ;

/* expressions */

opt_expr: NOINIT { $$ = NULL; }
    | expr { $$ = $1; }
    ;

opt_expr_list: %empty { $$ = &kEmptyVector; }
    | expr_list { $$ = $1; }
    ;

expr_list: expr { $$ = ctx_vector_init($1, x); }
    | expr_list COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

primary_expr: LPAREN expr RPAREN { $$ = $2; }
    | INTEGER { $$ = ctu_expr_int(x, @$, $1); }
    | BOOLEAN { $$ = ctu_expr_bool(x, @$, $1); }
    | STRING { $$ = ctu_expr_string(x, @$, $1.text, $1.length); }
    | path { $$ = ctu_expr_name(x, @$, $1); }
    | AS LT type GT LPAREN expr RPAREN { $$ = ctu_expr_cast(x, @$, $6, $3); }
    | init { $$ = $1; }
    ;

postfix_expr: primary_expr { $$ = $1; }
    | postfix_expr LPAREN opt_expr_list RPAREN { $$ = ctu_expr_call(x, @$, $1, $3); }
    | postfix_expr LSQUARE expr RSQUARE { $$ = ctu_expr_index(x, @$, $1, $3); }
    | postfix_expr DOT IDENT { $$ = ctu_expr_field(x, @$, $1, $3); }
    | postfix_expr ARROW IDENT { $$ = ctu_expr_field_indirect(x, @$, $1, $3); }
    ;

unary_expr: postfix_expr { $$ = $1; }
    | MINUS unary_expr { $$ = ctu_expr_unary(x, @$, eUnaryNeg, $2); }
    | PLUS unary_expr { $$ = ctu_expr_unary(x, @$, eUnaryAbs, $2); }
    | NOT unary_expr { $$ = ctu_expr_unary(x, @$, eUnaryNot, $2); }
    | STAR unary_expr { $$ = ctu_expr_deref(x, @$, $2); }
    | BITAND unary_expr { $$ = ctu_expr_ref(x, @$, $2); }
    ;

mul_expr: unary_expr { $$ = $1; }
    | unary_expr STAR mul_expr { $$ = ctu_expr_binary(x, @$, eBinaryMul, $1, $3); }
    | unary_expr DIVIDE mul_expr { $$ = ctu_expr_binary(x, @$, eBinaryDiv, $1, $3); }
    | unary_expr MODULO mul_expr { $$ = ctu_expr_binary(x, @$, eBinaryRem, $1, $3); }
    ;

add_expr: mul_expr { $$ = $1; }
    | mul_expr PLUS add_expr { $$ = ctu_expr_binary(x, @$, eBinaryAdd, $1, $3); }
    | mul_expr MINUS add_expr { $$ = ctu_expr_binary(x, @$, eBinarySub, $1, $3); }
    ;

shift_expr: add_expr { $$ = $1; }
    | add_expr SHL shift_expr { $$ = ctu_expr_binary(x, @$, eBinaryShl, $1, $3); }
    | add_expr SHR shift_expr { $$ = ctu_expr_binary(x, @$, eBinaryShr, $1, $3); }
    ;

bit_expr: shift_expr { $$ = $1; }
    | shift_expr BITAND bit_expr { $$ = ctu_expr_binary(x, @$, eBinaryBitAnd, $1, $3); }
    | shift_expr BITOR bit_expr { $$ = ctu_expr_binary(x, @$, eBinaryBitOr, $1, $3); }
    ;

xor_expr: bit_expr { $$ = $1; }
    | bit_expr BITXOR xor_expr { $$ = ctu_expr_binary(x, @$, eBinaryXor, $1, $3); }
    ;

compare_expr: xor_expr { $$ = $1; }
    | xor_expr LT compare_expr { $$ = ctu_expr_compare(x, @$, eCompareLt, $1, $3); }
    | xor_expr GT compare_expr { $$ = ctu_expr_compare(x, @$, eCompareGt, $1, $3); }
    | xor_expr LTE compare_expr { $$ = ctu_expr_compare(x, @$, eCompareLte, $1, $3); }
    | xor_expr GTE compare_expr { $$ = ctu_expr_compare(x, @$, eCompareGte, $1, $3); }
    ;

eq_expr: compare_expr { $$ = $1; }
    | eq_expr EQ compare_expr { $$ = ctu_expr_compare(x, @$, eCompareEq, $1, $3); }
    | eq_expr NEQ compare_expr { $$ = ctu_expr_compare(x, @$, eCompareNeq, $1, $3); }
    ;

and_expr: eq_expr { $$ = $1; }
    | eq_expr AND xor_expr { $$ = ctu_expr_compare(x, @$, eCompareAnd, $1, $3); }
    ;

or_expr: and_expr { $$ = $1; }
    | or_expr OR and_expr { $$ = ctu_expr_compare(x, @$, eCompareOr, $1, $3); }
    ;

expr: or_expr { $$ = $1; }
    ;

/* basic */

opt_ident: %empty { $$ = NULL; }
    | IDENT { $$ = $1; }
    ;

ident: IDENT { $$ = $1; }
    | DISCARD { $$ = NULL; }
    ;

path: IDENT { $$ = ctx_vector_init($1, x); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

%%
