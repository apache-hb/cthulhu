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
    #include "ast.h"
    #include "scan.h"
    #define YYSTYPE CTUSTYPE
    #define YYLTYPE CTULTYPE
}

%{
int ctulex(void *lval, void *loc, scan_t *scan);
void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    bool boolean;
    char *ident;
    
    string_t string;

    struct {
        mpz_t mpz;
        char *suffix;
    } digit;

    mpq_t decimal;

    ast_t *ast;
    vector_t *vector;
}

%token<ident>
    IDENT "identifier"

%token<string>
    STRING "string literal"

%token<digit>
    INTEGER "integer literal"

%token<boolean>
    BOOLEAN "boolean literal"

%token<decimal>
    DECIMAL "decimal literal"

%token
    DISCARD "`$`"
    BITNOT "`~`"
    AT "`@`"
    COLON "`:`"
    COLON2 "`::`"
    SEMICOLON "`;`"
    COMMA "`,`"
    DOT "`.`"
    DOT3 "`...`"
    ARROW "`->`"
    LSHIFT "`<<`"
    RSHIFT "`>>`"

    BEGIN_TEMPLATE "`!<` (template)"
    END_TEMPLATE "`>` (template)"

    NOT "`!`"
    EQUALS "`=`"
    EQ "`==`"
    NEQ "`!=`"

    ADD "`+`"
    SUB "`-`"
    MUL "`*`"
    DIV "`/`"
    MOD "`%`"

    AND "`&&`"
    OR "`||`"

    BITAND "`&`"
    BITOR "`|`"
    BITXOR "`^`"

    LPAREN "`(`"
    RPAREN "`)`"
    LSQUARE "`[`"
    RSQUARE "`]`"
    LBRACE "`{`"
    RBRACE "`}`"

    GT "`>`"
    GTE "`>=`"
    LT "`<`"
    LTE "`<=`"

    MODULE "`module`"
    IMPORT "`import`"
    EXPORT "`export`"
    USING "`using`"

    DEF "`def`"
    VAR "`var`"
    FINAL "`final`"
    CONST "`const`"

    OBJECT "`object`"
    STRUCT "`struct`"
    UNION "`union`"
    VARIANT "`variant`"
    TYPE "`type`"

    IF "`if`"
    ELSE "`else`"
    WHILE "`while`"
    FOR "`for`"
    BREAK "`break`"
    CONTINUE "`continue`"
    RETURN "`return`"

    NEW "`new`"
    DELETE "`delete`"

    LAMBDA "`lambda`"
    AS "`as`"

    NIL "`null`"
    NOINIT "`noinit`"
    INIT "`init`"

    MATCH "`match`"
    DEFAULT "`default`"
    CASE "`case`"
    SELECT "`select`"
    ORDER "`order`"
    ON "`on`"
    FROM "`from`"
    WHERE "`where`"
    WHEN "`when`"
    THEN "`then`"
    WITH "`with`"

    IN "`in`"
    OUT "`out`"
    INOUT "`inout`"

    PRIVATE "`private`"
    PUBLIC "`public`"
    PROTECTED "`protected`"

    ASYNC "`async`"
    AWAIT "`await`"
    YIELD "`yield`"

    UNIQUE "`unique`"
    SHARED "`shared`"
    STRONG "`strong`"
    WEAK "`weak`"
    MOVE "`move`"

    CONTRACT "`contract`"
    REQUIRES "`requires`"
    ASSERT "`assert`"
    ENSURES "`ensures`"
    INVARIANT "`invariant`"
    ALWAYS "`always`"

    SIZEOF "`sizeof`"
    ALIGNOF "`alignof`"
    OFFSETOF "`offsetof`"
    TYPEOF "`typeof`"
    UUIDOF "`uuidof`"

    TRY "`try`"
    CATCH "`catch`"
    FINALLY "`finally`"
    THROW "`throw`"

    COMPILED "`compile`"
    STATIC "`static`"
    EXPLICIT "`explicit`"
    INLINE "`inline`"

%type<ast>
    modspec decl innerDecl
    structdecl uniondecl variantdecl aliasdecl
    field variant newtype
    funcdecl funcresult funcbody funcsig
    vardecl
    param
    stmt stmts
    type import
    expr postfix unary multiply add 
    compare equality shift bits xor and or
    primary else
    varinit vartype
    branch elif
    init underlying
    opttype
    fieldinit
    caseField

%type<vector>
    path decls decllist
    fieldlist aggregates
    imports importlist
    variants variantlist
    stmtlist paramlist
    args attrargs argbody
    funcparams opttypes types
    optinits inits
    caseFields caseData items

%type<ident>
    label block ident

%type<boolean>
    mut optExport defaultCase

%start program

%%

program: modspec imports decls { scan_set(x, ast_program(x, @$, $1, $2, $3)); }
    ;

imports: %empty { $$ = vector_new(0); }
    | importlist { $$ = $1; }
    ;

importlist: import { $$ = vector_init($1); }
    | importlist import { vector_push(&$1, $2); $$ = $1; }
    ;

import: IMPORT path SEMICOLON { $$ = ast_import(x, @$, $2, NULL, NULL); }
    | IMPORT path AS IDENT SEMICOLON { $$ = ast_import(x, @$, $2, $4, NULL); }
    | IMPORT path LPAREN items RPAREN SEMICOLON { $$ = ast_import(x, @$, $2, NULL, $4); }
    | IMPORT path LPAREN DOT3 RPAREN SEMICOLON { $$ = ast_import(x, @$, $2, NULL, vector_of(0)); }
    ;

items: IDENT { $$ = vector_init($1); }
    | items COMMA IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

decls: %empty { $$ = vector_new(0); }
    | decllist { $$ = $1; }
    ;

decllist: decl { $$ = vector_init($1); }
    | decllist decl { vector_push(&$1, $2); $$ = $1; }
    ;

decl: optAttribs optExport innerDecl { set_attribs($3, $2, collect_attributes(x)); $$ = $3; }
    ;

optExport: %empty { $$ = false; }
    | EXPORT { $$ = true; }
    ;

optAttribs: %empty | attribs ;

attribs: attrib | attribs attrib ;

attrib: AT attribBody
    | AT attribList
    ;
    
attribList: LSQUARE attribBodyList RSQUARE
    ;

attribBodyList: attribBody
    | attribBodyList COMMA attribBody
    ;

attribBody: IDENT attrargs { add_attribute(x, ast_attribute(x, @$, $1, $2)); } 
    ;

attrargs: %empty { $$ = vector_new(0); }
    | LPAREN args RPAREN { $$ = $2; }
    ;

innerDecl: structdecl { $$ = $1; }
    | uniondecl { $$ = $1; }
    | aliasdecl { $$ = $1; }
    | variantdecl { $$ = $1; }
    | funcdecl { $$ = $1; }
    | vardecl { $$ = $1; }
    | newtype SEMICOLON { $$ = $1; }
    ;

funcdecl: DEF IDENT funcsig funcbody { $$ = ast_function(x, @$, $2, $3, $4); }
    ;

vardecl: mut IDENT vartype EQUALS varinit SEMICOLON { $$ = ast_variable(x, @$, $2, $1, $3, $5); }
    ;

vartype: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

varinit: expr { $$ = $1; }
    | NOINIT { $$ = NULL; }
    ;

mut: VAR { $$ = true; }
    | CONST { $$ = false; }
    ;

funcsig: funcparams funcresult { $$ = ast_closure(x, @$, $1, $2); }
    ;

funcresult: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

funcparams: %empty { $$ = vector_of(0); }
    | LPAREN paramlist RPAREN { $$ = $2; }
    ;

paramlist: param { $$ = vector_init($1); }
    | paramlist COMMA param { vector_push(&$1, $3); $$ = $1; }
    ;

param: ident COLON type { $$ = ast_param(x, @$, $1, $3); }
    ;

funcbody: SEMICOLON { $$ = NULL; }
    | EQUALS expr SEMICOLON { $$ = ast_stmts(x, @$, vector_init(ast_return(x, @2, $2))); }
    | stmts { $$ = $1; }
    ;

aliasdecl: TYPE IDENT EQUALS type SEMICOLON { $$ = ast_typealias(x, @$, $2, $4); }
    ;

structdecl: STRUCT IDENT aggregates { $$ = ast_structdecl(x, @$, $2, $3); }
    ;

uniondecl: UNION IDENT aggregates { $$ = ast_uniondecl(x, @$, $2, $3); }
    ;

variantdecl: VARIANT IDENT underlying LBRACE variants RBRACE { $$ = ast_variantdecl(x, @$, $2, $3, $5); }
    ;

newtype: VARIANT IDENT EQUALS type { $$ = ast_newtype(x, @$, $2, $4); }
    ;

underlying: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

variants: %empty { $$ = vector_new(0); }
    | variantlist { $$ = $1; }
    ;

variantlist: variant { $$ = vector_init($1); }
    | variantlist variant { vector_push(&$1, $2); $$ = $1; }
    ;

variant: defaultCase[isDefault] IDENT[name] caseData[fields] init[value] { $$ = ast_case(x, @$, $name, $fields, $value, $isDefault); }
    ;

defaultCase: DEFAULT { $$ = true; }
    | CASE { $$ = false; }
    ;

caseData: %empty { $$ = vector_of(0); }
    | LPAREN caseFields RPAREN { $$ = $2; }
    ;

caseFields: caseField { $$ = vector_init($1); }
    | caseFields COMMA caseField { vector_push(&$1, $3); $$ = $1; }
    ;

caseField: ident COLON type { $$ = ast_param(x, @$, $1, $3); }
    ;

init: %empty { $$ = NULL; }
    | EQUALS expr { $$ = $2; }
    ;

aggregates: LBRACE fieldlist RBRACE { $$ = $2; }
    ;

fieldlist: field { $$ = vector_init($1); }
    | fieldlist field { vector_push(&$1, $2); $$ = $1; }
    ;

field: ident COLON type SEMICOLON { $$ = ast_field(x, @$, $1, $3, NULL); }
    ;

modspec: %empty { $$ = NULL; }
    | MODULE path SEMICOLON { $$ = ast_module(x, @$, $2); }
    ;

stmtlist: stmt { $$ = vector_init($1); }
    | stmtlist stmt { vector_push(&$1, $2); $$ = $1; }
    ;

stmts: LBRACE RBRACE { $$ = ast_stmts(x, @$, vector_of(0)); }
    | LBRACE stmtlist RBRACE { $$ = ast_stmts(x, @$, $2); }
    ;

stmt: stmts { $$ = $1; }
    | RETURN expr SEMICOLON { $$ = ast_return(x, @$, $2); }
    | block WHILE expr stmts else { $$ = ast_while(x, @$, $1, $3, $4, $5); }
    | BREAK label SEMICOLON { $$ = ast_break(x, @$, $2); }
    | CONTINUE label SEMICOLON { $$ = ast_continue(x, @$, $2); }
    | vardecl { $$ = $1; }
    | expr SEMICOLON { $$ = $1; }
    | branch { $$ = $1; }
    | expr EQUALS expr SEMICOLON { $$ = ast_assign(x, @$, $1, $3); }
    ;

label: %empty { $$ = NULL; }
    | IDENT { $$ = $1; }
    ;

block: %empty { $$ = NULL; }
    | IDENT COLON { $$ = $1; }
    ;

branch: IF expr stmts elif { $$ = ast_branch(x, @$, $2, $3, $4); }
    ;

elif: %empty { $$ = NULL; }
    | ELSE branch { $$ = $2;}
    | ELSE stmts { $$ = $2; }
    ;

else: %empty { $$ = NULL; }
    | ELSE stmts { $$ = $2; }
    ;

type: path { $$ = ast_typename(x, @$, $1); }
    | MUL type { $$ = ast_pointer(x, @$, $2, false); }
    | LSQUARE MUL RSQUARE type { $$ = ast_pointer(x, @$, $4, true); }
    | LSQUARE expr RSQUARE type { $$ = ast_array(x, @$, $2, $4); }
    | DEF LPAREN opttypes RPAREN ARROW type { $$ = ast_closure(x, @$, $3, $6); } 
    | LPAREN type RPAREN { $$ = $2; }
    | DOT3 { $$ = ast_varargs(x, @$); }
    ;

opttypes: %empty { $$ = vector_of(0); }
    | types { $$ = $1; }
    ;

types: type { $$ = vector_init($1); }
    | types COMMA type { vector_push(&$1, $3); $$ = $1; }
    ;

primary: LPAREN expr RPAREN { $$ = $2; }
    | INTEGER { $$ = ast_digit(x, @$, $1.mpz, $1.suffix); }
    | DECIMAL { $$ = ast_decimal(x, @$, $1); }
    | path { $$ = ast_name(x, @$, $1); }
    | BOOLEAN { $$ = ast_bool(x, @$, $1); }
    | STRING { $$ = ast_string(x, @$, $1.text, $1.size); }
    | NIL { $$ = ast_null(x, @$); }
    | SIZEOF LPAREN type RPAREN { $$ = ast_sizeof(x, @$, $3); }
    | DOT opttype LBRACE optinits RBRACE { $$ = ast_init(x, @$, $2, $4); }
    | DEFAULT { $$ = ast_default(x, @$); }
    ;

opttype: %empty { $$ = NULL; }
    | type { $$ = $1; }
    ;

optinits: %empty { $$ = vector_of(0); }
    | inits { $$ = $1; }
    ;

inits: fieldinit { $$ = vector_init($1); }
    | inits COMMA fieldinit { vector_push(&$1, $3); $$ = $1; }
    ;

fieldinit: IDENT EQUALS expr { $$ = ast_fieldinit(x, @$, $1, $3); }
    ;

postfix: primary { $$ = $1; }
    | postfix LPAREN args RPAREN { $$ = ast_call(x, @$, $1, $3); }
    | postfix DOT IDENT { $$ = ast_access(x, @$, $1, $3, false); }
    | postfix ARROW IDENT { $$ = ast_access(x, @$, $1, $3, true); }
    | postfix LSQUARE expr RSQUARE { $$ = ast_index(x, @$, $1, $3); }
    ;

unary: postfix { $$ = $1; }
    | SUB unary { $$ = ast_unary(x, @$, eUnaryNeg, $2); }
    | ADD unary { $$ = ast_unary(x, @$, eUnaryAbs, $2); }
    | BITNOT unary { $$ = ast_unary(x, @$, eUnaryFlip, $2); }
    | NOT unary { $$ = ast_unary(x, @$, eUnaryNot, $2); }
    | BITAND unary { $$ = ast_ref(x, @$, $2); }
    | MUL unary { $$ = ast_deref(x, @$, $2); }
    ;

multiply: unary { $$ = $1; }
    | multiply MUL unary { $$ = ast_binary(x, @$, eBinaryMul, $1, $3); }
    | multiply DIV unary { $$ = ast_binary(x, @$, eBinaryDiv, $1, $3); }
    | multiply MOD unary { $$ = ast_binary(x, @$, eBinaryRem, $1, $3); }
    ;

add: multiply { $$ = $1; }
    | add ADD multiply { $$ = ast_binary(x, @$, eBinaryAdd, $1, $3); }
    | add SUB multiply { $$ = ast_binary(x, @$, eBinarySub, $1, $3); }
    ;

compare: add { $$ = $1; }
    | compare LT add { $$ = ast_compare(x, @$, eCompareLt, $1, $3); }
    | compare GT add { $$ = ast_compare(x, @$, eCompareGt, $1, $3); }
    | compare LTE add { $$ = ast_compare(x, @$, eCompareLte, $1, $3); }
    | compare GTE add { $$ = ast_compare(x, @$, eCompareGte, $1, $3); }
    ;

equality: compare { $$ = $1; }
    | equality EQ compare { $$ = ast_compare(x, @$, eCompareEq, $1, $3); }
    | equality NEQ compare { $$ = ast_compare(x, @$, eCompareNeq, $1, $3); }
    ;

shift: equality { $$ = $1; }
    | shift RSHIFT equality { $$ = ast_binary(x, @$, eBinaryShl, $1, $3); }
    | shift LSHIFT equality { $$ = ast_binary(x, @$, eBinaryShr, $1, $3); }
    ;

bits: shift { $$ = $1; }
    | bits BITAND shift { $$ = ast_binary(x, @$, eBinaryBitAnd, $1, $3); }
    | bits BITOR shift { $$ = ast_binary(x, @$, eBinaryBitOr, $1, $3); }
    ;

xor: bits { $$ = $1; }
    | xor BITXOR bits { $$ = ast_binary(x, @$, eBinaryXor, $1, $3); }
    ;

and: xor { $$ = $1; }
    | and AND xor { $$ = ast_compare(x, @$, eCompareAnd, $1, $3); }
    ;

or: and { $$ = $1; }
    | or OR and { $$ = ast_compare(x, @$, eCompareOr, $1, $3); }
    ;

expr: or { $$ = $1; }
    ;

argbody: expr { $$ = vector_init($1); }
    | argbody COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

args: %empty { $$ = vector_new(0); }
    | argbody { $$ = $1; } 
    ; 

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

ident: IDENT { $$ = $1; } 
    | DISCARD { $$ = NULL; }
    ;

%%
