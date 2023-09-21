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
    util_text_t string;
    ctu_digit_t digit;
    bool boolean;

    vector_t *vector;

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
    path modspec
    imports importList
    decls declList
    structFields
    stmtList
    fnParamList
    attribArgs
    exprList optExprList
    typeList optTypeList
    variantFields optVariantFields
    initList
    attribs attrib attribBody attribList

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
    decl innerDecl globalDecl functionDecl structDecl structField typeAliasDecl
    functionBody
    type
    primary expr
    orExpr andExpr eqExpr cmpExpr xorExpr bitExpr shiftExpr addExpr mulExpr unaryExpr postExpr
    maybeExpr
    stmt stmts localDecl returnStmt whileStmt assignStmt branchStmt
    fnParam fnResult
    variantDecl variantField underlying
    fieldInit init
    singleAttrib

%type<ident>
    importAlias ident optIdent whileName variadic

%type<boolean>
    exported mut isDefault

%type<params>
    fnParams

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

/* decorators */

attribs: %empty { $$ = vector_of(0); }
    | attribs attrib { vector_append(&$1, $2); $$ = $1; }
    ;

attrib: AT attribBody { $$ = $2; }
    ;

attribBody: singleAttrib { $$ = vector_init($1); }
    | LSQUARE attribList RSQUARE { $$ = $2; }
    ;

attribList: singleAttrib { $$ = vector_init($1); }
    | attribList COMMA singleAttrib { vector_push(&$1, $3); $$ = $1; }
    ;

singleAttrib: path { $$ = ctu_attrib(x, @$, $1, vector_of(0)); }
    | path LPAREN RPAREN { $$ = ctu_attrib(x, @$, $1, vector_of(0)); }
    | path LPAREN attribArgs RPAREN { $$ = ctu_attrib(x, @$, $1, $3); }
    ;

attribArgs: expr { $$ = vector_init($1); }
    | attribArgs COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

/* toplevel decls */

decls: %empty { $$ = vector_of(0); }
    | declList { $$ = $1; }
    ;

declList: decl { $$ = vector_init($1); }
    | declList decl { vector_push(&$1, $2); $$ = $1; }
    ;

decl: attribs innerDecl { $$ = ctu_apply($2, $1); }
    ;

innerDecl: globalDecl { $$ = $1; }
    | functionDecl { $$ = $1; }
    | structDecl { $$ = $1; }
    | typeAliasDecl { $$ = $1; }
    | variantDecl { $$ = $1; }
    ;

/* variants/enums */

variantDecl: exported VARIANT IDENT underlying LBRACE optVariantFields RBRACE { $$ = ctu_decl_variant(x, @$, $1, $3, $4, $6); }
    ;

underlying: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

optVariantFields: %empty { $$ = vector_of(0); }
    | variantFields { $$ = $1; }
    ;

variantFields: variantField { $$ = vector_init($1); }
    | variantFields variantField { vector_push(&$1, $2); $$ = $1; }
    ;

variantField: isDefault ident ASSIGN expr { $$ = ctu_variant_case(x, @$, $2, $1, $4); }
    ;

isDefault: DEFAULT { $$ = true; }
    | CASE { $$ = false; }
    ;

/* functions */

functionDecl: exported DEF IDENT fnParams fnResult functionBody { $$ = ctu_decl_function(x, @$, $1, $3, $4.params, $4.variadic, $5, $6); }
    ;

fnResult: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

fnParams: %empty { $$ = ctu_params_new(vector_of(0), NULL); }
    | LPAREN fnParamList variadic RPAREN { $$ = ctu_params_new($2, $3); }
    ;

variadic: %empty { $$ = NULL; }
    | COMMA IDENT COLON DOT3 { $$ = $2; }
    ;

fnParamList: fnParam { $$ = vector_init($1); }
    | fnParamList COMMA fnParam { vector_push(&$1, $3); $$ = $1; }
    ;

fnParam: IDENT COLON type { $$ = ctu_param(x, @$, $1, $3); }
    ;

functionBody: ASSIGN expr SEMI { $$ = ctu_stmt_return(x, @$, $2); }
    | stmts { $$ = $1; }
    | SEMI { $$ = NULL; }
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
    | LSQUARE STAR RSQUARE type { $$ = ctu_type_pointer(x, @$, $4); /* TODO: implement indexable pointers */ }
    | DEF LPAREN optTypeList RPAREN ARROW type { $$ = ctu_type_function(x, @$, $3, $6); }
    | LSQUARE expr RSQUARE type { $$ = ctu_type_array(x, @$, $4, $2); }
    ;

typeList: type { $$ = vector_init($1); }
    | typeList COMMA type { vector_push(&$1, $3); $$ = $1; }
    ;

optTypeList: %empty { $$ = vector_of(0); }
    | typeList { $$ = $1; }
    ;

/* statements */

stmtList: %empty { $$ = vector_of(0); }
    | stmtList stmt { vector_push(&$1, $2); $$ = $1; }
    ;

whileName: %empty { $$ = NULL; }
    | COLON2 IDENT { $$ = $2; }
    ;

whileStmt: WHILE whileName expr stmts { $$ = ctu_stmt_while(x, @$, $2, $3, $4, NULL); }
    | WHILE whileName expr stmts ELSE stmts { $$ = ctu_stmt_while(x, @$, $2, $3, $4, $6); }
    ;

returnStmt: RETURN expr SEMI { $$ = ctu_stmt_return(x, @$, $2); }
    | RETURN SEMI { $$ = ctu_stmt_return(x, @$, NULL); }
    ;

stmts: LBRACE stmtList RBRACE { $$ = ctu_stmt_list(x, @$, $2); }
    ;

localDecl: mut IDENT COLON type ASSIGN maybeExpr SEMI { $$ = ctu_stmt_local(x, @$, $1, $2, $4, $6); }
    | mut IDENT ASSIGN expr SEMI { $$ = ctu_stmt_local(x, @$, $1, $2, NULL, $4); }
    ;

assignStmt: expr ASSIGN expr SEMI { $$ = ctu_stmt_assign(x, @$, $1, $3); }
    ;

branchStmt: IF expr stmts { $$ = ctu_stmt_branch(x, @$, $2, $3, NULL); }
    | IF expr stmts ELSE stmts { $$ = ctu_stmt_branch(x, @$, $2, $3, $5); }
    ;

stmt: expr SEMI { $$ = $1; }
    | stmts { $$ = $1; }
    | returnStmt { $$ = $1; }
    | localDecl { $$ = $1; }
    | whileStmt { $$ = $1; }
    | assignStmt { $$ = $1; }
    | branchStmt { $$ = $1; }
    | BREAK optIdent SEMI { $$ = ctu_stmt_break(x, @$, $2); }
    | CONTINUE optIdent SEMI { $$ = ctu_stmt_continue(x, @$, $2); }
    ;

/* init */

init: DOT LBRACE initList RBRACE { $$ = ctu_expr_init(x, @$, $3); }
    ;

initList: fieldInit { $$ = vector_init($1); }
    | initList COMMA fieldInit { vector_push(&$1, $3); $$ = $1; }
    ;

fieldInit: ident ASSIGN expr { $$ = ctu_field_init(x, @$, $1, $3); }
    ;

/* expressions */

maybeExpr: NOINIT { $$ = NULL; }
    | expr { $$ = $1; }
    ;

optExprList: %empty { $$ = vector_of(0); }
    | exprList { $$ = $1; }
    ;

exprList: expr { $$ = vector_init($1); }
    | exprList COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

primary: LPAREN expr RPAREN { $$ = $2; }
    | INTEGER { $$ = ctu_expr_int(x, @$, $1.value); }
    | BOOLEAN { $$ = ctu_expr_bool(x, @$, $1); }
    | STRING { $$ = ctu_expr_string(x, @$, $1.text, $1.length); }
    | path { $$ = ctu_expr_name(x, @$, $1); }
    | AS LT type GT LPAREN primary RPAREN { $$ = ctu_expr_cast(x, @$, $6, $3); }
    | init { $$ = $1; }
    ;

postExpr: primary { $$ = $1; }
    | postExpr LPAREN optExprList RPAREN { $$ = ctu_expr_call(x, @$, $1, $3); }
    | postExpr LSQUARE expr RSQUARE { $$ = ctu_expr_index(x, @$, $1, $3); }
    | postExpr DOT IDENT { $$ = ctu_expr_field(x, @$, $1, $3); }
    | postExpr ARROW IDENT { $$ = ctu_expr_field_indirect(x, @$, $1, $3); }
    ;

unaryExpr: postExpr { $$ = $1; }
    | MINUS unaryExpr { $$ = ctu_expr_unary(x, @$, eUnaryNeg, $2); }
    | PLUS unaryExpr { $$ = ctu_expr_unary(x, @$, eUnaryAbs, $2); }
    | NOT unaryExpr { $$ = ctu_expr_unary(x, @$, eUnaryNot, $2); }
    | STAR unaryExpr { $$ = ctu_expr_deref(x, @$, $2); }
    | BITAND unaryExpr { $$ = ctu_expr_ref(x, @$, $2); }
    ;

mulExpr: unaryExpr { $$ = $1; }
    | unaryExpr STAR mulExpr { $$ = ctu_expr_binary(x, @$, eBinaryMul, $1, $3); }
    | unaryExpr DIVIDE mulExpr { $$ = ctu_expr_binary(x, @$, eBinaryDiv, $1, $3); }
    | unaryExpr MODULO mulExpr { $$ = ctu_expr_binary(x, @$, eBinaryRem, $1, $3); }
    ;

addExpr: mulExpr { $$ = $1; }
    | mulExpr PLUS addExpr { $$ = ctu_expr_binary(x, @$, eBinaryAdd, $1, $3); }
    | mulExpr MINUS addExpr { $$ = ctu_expr_binary(x, @$, eBinarySub, $1, $3); }
    ;

shiftExpr: addExpr { $$ = $1; }
    | addExpr SHL shiftExpr { $$ = ctu_expr_binary(x, @$, eBinaryShl, $1, $3); }
    | addExpr SHR shiftExpr { $$ = ctu_expr_binary(x, @$, eBinaryShr, $1, $3); }
    ;

bitExpr: shiftExpr { $$ = $1; }
    | shiftExpr BITAND bitExpr { $$ = ctu_expr_binary(x, @$, eBinaryBitAnd, $1, $3); }
    | shiftExpr BITOR bitExpr { $$ = ctu_expr_binary(x, @$, eBinaryBitOr, $1, $3); }
    ;

xorExpr: bitExpr { $$ = $1; }
    | bitExpr BITXOR xorExpr { $$ = ctu_expr_binary(x, @$, eBinaryXor, $1, $3); }
    ;

cmpExpr: xorExpr { $$ = $1; }
    | xorExpr LT cmpExpr { $$ = ctu_expr_compare(x, @$, eCompareLt, $1, $3); }
    | xorExpr GT cmpExpr { $$ = ctu_expr_compare(x, @$, eCompareGt, $1, $3); }
    | xorExpr LTE cmpExpr { $$ = ctu_expr_compare(x, @$, eCompareLte, $1, $3); }
    | xorExpr GTE cmpExpr { $$ = ctu_expr_compare(x, @$, eCompareGte, $1, $3); }
    ;

eqExpr: cmpExpr { $$ = $1; }
    | eqExpr EQ cmpExpr { $$ = ctu_expr_compare(x, @$, eCompareEq, $1, $3); }
    | eqExpr NEQ cmpExpr { $$ = ctu_expr_compare(x, @$, eCompareNeq, $1, $3); }
    ;

andExpr: eqExpr { $$ = $1; }
    | eqExpr AND xorExpr { $$ = ctu_expr_compare(x, @$, eCompareAnd, $1, $3); }
    ;

orExpr: andExpr { $$ = $1; }
    | orExpr OR andExpr { $$ = ctu_expr_compare(x, @$, eCompareOr, $1, $3); }
    ;

expr: orExpr { $$ = $1; }
    ;

/* basic */

optIdent: %empty { $$ = NULL; }
    | IDENT { $$ = $1; }
    ;

ident: IDENT { $$ = $1; }
    | DISCARD { $$ = NULL; }
    ;

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

%%