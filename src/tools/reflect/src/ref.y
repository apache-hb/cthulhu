%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ref}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"
}

%code requires {
    #include "std/vector.h"
    #include "ref/ast.h"
    #include "ref/scan.h"
    #define YYSTYPE REFSTYPE
    #define YYLTYPE REFLTYPE
}

%{
int reflex(void *lval, void *loc, scan_t *scan);
void referror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    ref_ast_t *ast;
    vector_t *vector;
    typevec_t *typevec;

    ref_param_t param;
    ref_flags_t flags;
    ref_attrib_tag_t attrib;
    ref_privacy_t privacy;

    text_t string;
    bool boolean;
    char *ident;
    mpz_t integer;
}

%type<ast>
    decl decl_body
    class_body_item
    class_decl
    type
    opt_assign

    expr
    variant_decl
    case_item
    struct_decl
    opt_inherit
    attrib
    class_body_inner

    primary_expression
    postfix_expression
    unary_expression
    multiplicative_expression
    additive_expression
    shift_expression
    relational_expression
    equality_expression
    and_expression
    exclusive_or_expression
    inclusive_or_expression
    logical_and_expression
    logical_or_expression
    opt_return_type
    param
    import
    case_item_inner
    case_value
    method_decl
    ctor_decl

%type<vector>
    path
    decl_seq
    class_body_seq
    case_seq
    opt_attrib_seq
    attrib_seq
    type_list
    ident_list
    opt_tparams
    opt_params
    param_list
    opt_import_seq
    import_seq

%type<typevec> string_list
%type<param> passing
%type<flags> opt_decl_flags_seq decl_flags_seq decl_flag
%type<attrib> simple_attrib layout_attrib
%type<privacy> privacy_spec

%type<ident>
    opt_api

%token<ident> TOK_IDENT "identifier"
%token<integer> TOK_INTEGER "integer literal"
%token<string> TOK_STRING "string literal"

%token
    TOK_MODULE "module"
    TOK_IMPORT "import"

    TOK_CLASS "class"
    TOK_STRUCT "struct"
    TOK_VIRTUAL "virtual"
    TOK_VARIANT "variant"
    TOK_ALIAS "alias"

    TOK_PUBLIC "public"
    TOK_PRIVATE "private"
    TOK_PROTECTED "protected"

    TOK_EXPORT "export"

    TOK_CONST "const"
    TOK_DEF "def"

    TOK_NEW "new"
    TOK_DELETE "delete"
    TOK_DEFAULT "default"

    TOK_LBRACE "{"
    TOK_RBRACE "}"

    TOK_LPAREN "("
    TOK_RPAREN ")"

    TOK_LSQUARE "["
    TOK_RSQUARE "]"

    TOK_AND "&&"
    TOK_OR "||"

    TOK_LSHIFT "<<"
    TOK_RSHIFT ">>"

    TOK_LTE "<="
    TOK_LT "<"

    TOK_GTE ">="
    TOK_GT ">"
    TOK_COMMA ","

    TOK_MUL "*"
    TOK_DIV "/"
    TOK_MOD "%"
    TOK_MINUS "-"
    TOK_PLUS "+"
    TOK_XOR "^"
    TOK_NOT "!"

    TOK_BITAND "&"
    TOK_BITOR "|"

    TOK_EQ "=="
    TOK_NEQ "!="

    TOK_COLON2 "::"
    TOK_COLON ":"
    TOK_SEMICOLON ";"
    TOK_DOT "."
    TOK_ASSIGN "="

    TOK_BEGIN_ATTRIBUTE "[["
    TOK_END_ATTRIBUTE "]]"

    TOK_TRANSIENT "transient"
    TOK_CONFIG "config"
    TOK_CATEGORY "category"
    TOK_BRIEF "brief"
    TOK_ASSERT "assert"
    TOK_DEPRECATED "deprecated"
    TOK_TYPEID "typeid"
    TOK_LAYOUT "layout"
    TOK_SYSTEM "system"
    TOK_BITFLAGS "bitflags"
    TOK_ALIGNAS "alignas"
    TOK_PACKED "packed"
    TOK_ARITHMATIC "arithmatic"
    TOK_ITERATOR "iterator"
    TOK_CXXNAME "cxxname"
    TOK_REMOTE "remote"
    TOK_OPAQUE "opaque"
    TOK_CBUFFER "cbuffer"
    TOK_FACADE "facade"
    TOK_API "api"
    TOK_INTERNAL "internal"
    TOK_EXTERNAL "external"
    TOK_ORDERED "ordered"

    TOK_NULL "null"
    TOK_TRUE "true"
    TOK_FALSE "false"
    TOK_SEALED "sealed"

    TOK_CASE "case"

    TOK_IN "in"
    TOK_OUT "out"
    TOK_INOUT "inout"

    TOK_BEGIN_TEMPLATE "!< (begin template)"
    TOK_END_TEMPLATE "> (end template)"

%start program

%%

program: TOK_MODULE path opt_api TOK_SEMICOLON opt_import_seq decl_seq { scan_set(x, ref_program(x, @$, $2, $3, $5, $6)); };

opt_api: %empty { $$ = NULL; }
    | TOK_API TOK_IDENT { $$ = $2; }
    ;

opt_import_seq: %empty { $$ = &kEmptyVector; }
    | import_seq { $$ = $1; }
    ;

import_seq: import { $$ = vector_init($1, BISON_ARENA(x)); }
    | import_seq import { vector_push(&$1, $2); $$ = $1; }
    ;

import: TOK_IMPORT TOK_STRING TOK_SEMICOLON { $$ = ref_import(x, @$, $2); }
    ;

decl_seq: decl { $$ = vector_init($1, BISON_ARENA(x)); }
    | decl_seq decl { vector_push(&$1, $2); $$ = $1; }
    ;

decl: opt_attrib_seq opt_decl_flags_seq decl_body {
        ref_set_attribs($3, $1);
        ref_set_flags($3, $2);
        $$ = $3;
    }
    ;

opt_decl_flags_seq: %empty { $$ = eDeclNone; }
    | decl_flags_seq { $$ = $1; }
    ;

decl_flags_seq: decl_flag { $$ = $1; }
    | decl_flags_seq decl_flag { $$ = $1 | $2; }
    ;

decl_flag: TOK_VIRTUAL { $$ = eDeclVirtual; }
    | TOK_SEALED { $$ = eDeclSealed; }
    | TOK_CONST { $$ = eDeclConst; }
    ;

decl_body: class_decl { $$ = $1; }
    | variant_decl { $$ = $1; }
    | struct_decl { $$ = $1; }
    ;

opt_inherit: %empty { $$ = NULL; }
    | TOK_COLON type { $$ = $2; }
    ;

struct_decl: TOK_STRUCT TOK_IDENT opt_tparams opt_inherit TOK_LBRACE class_body_seq TOK_RBRACE { $$ = ref_struct(x, @$, $2, $3, $4, $6); }
    ;

variant_decl: TOK_VARIANT TOK_IDENT opt_inherit TOK_LBRACE case_seq TOK_RBRACE { $$ = ref_variant(x, @$, $2, $3, $5); }
    ;

case_seq: case_item { $$ = vector_init($1, BISON_ARENA(x)); }
    | case_seq case_item { vector_push(&$1, $2); $$ = $1; }
    ;

case_item: opt_attrib_seq case_item_inner { ref_set_attribs($2, $1); $$ = $2; }
    ;

case_item_inner: TOK_CASE TOK_IDENT TOK_ASSIGN case_value { $$ = ref_case(x, @$, $2, $4, false); }
    | TOK_DEFAULT TOK_IDENT TOK_ASSIGN case_value { $$ = ref_case(x, @$, $2, $4, true); }
    | opt_decl_flags_seq method_decl TOK_SEMICOLON { ref_set_flags($2, $1); $$ = $2; }
    ;

case_value: expr { $$ = $1; }
    | TOK_OPAQUE TOK_LPAREN TOK_IDENT TOK_RPAREN { $$ = ref_opaque(x, @$, $3); }
    ;

class_decl: TOK_CLASS TOK_IDENT opt_tparams opt_inherit TOK_LBRACE class_body_seq TOK_RBRACE { $$ = ref_class(x, @$, $2, $3, $4, $6); }
    ;

opt_tparams: %empty { $$ = &kEmptyVector; }
    | TOK_BEGIN_TEMPLATE ident_list TOK_END_TEMPLATE { $$ = $2; }
    ;

ident_list: TOK_IDENT { $$ = vector_init($1, BISON_ARENA(x)); }
    | ident_list TOK_COMMA TOK_IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

class_body_seq: class_body_item { $$ = vector_init($1, BISON_ARENA(x)); }
    | class_body_seq class_body_item { vector_push(&$1, $2); $$ = $1; }
    ;

class_body_item: privacy_spec TOK_COLON { $$ = ref_privacy(x, @$, $1); }
    | opt_attrib_seq class_body_inner TOK_SEMICOLON { ref_set_attribs($2, $1); $$ = $2; }
    ;

class_body_inner: TOK_IDENT TOK_COLON type opt_assign { $$ = ref_field(x, @$, $1, $3, $4); }
    | opt_decl_flags_seq method_decl { ref_set_flags($2, $1); $$ = $2; }
    | ctor_decl { $$ = $1; }
    ;

privacy_spec: TOK_PRIVATE { $$ = ePrivacyPrivate; }
    | TOK_PROTECTED { $$ = ePrivacyProtected; }
    | TOK_PUBLIC { $$ = ePrivacyPublic; }
    ;

ctor_decl: TOK_NEW TOK_LPAREN TOK_RPAREN { $$ = ref_ctor(x, @$, &kEmptyVector, NULL); }
    ;

method_decl: TOK_DEF TOK_IDENT opt_params opt_return_type opt_assign { $$ = ref_method(x, @$, eDeclNone, $2, $3, $4, $5); }
    ;

opt_return_type: %empty { $$ = NULL; }
    | TOK_COLON type { $$ = $2; }
    ;

opt_params: TOK_LPAREN TOK_RPAREN { $$ = &kEmptyVector; }
    | TOK_LPAREN param_list TOK_RPAREN { $$ = $2; }
    ;

param_list: param { $$ = vector_init($1, BISON_ARENA(x)); }
    | param_list TOK_COMMA param { vector_push(&$1, $3); $$ = $1; }
    ;

passing: %empty { $$ = eParamIn; }
    | TOK_IN { $$ = eParamIn; }
    | TOK_OUT { $$ = eParamOut; }
    | TOK_BITAND { $$ = eParamInOut; }
    ;

param: passing TOK_IDENT TOK_COLON type { $$ = ref_param(x, @$, $2, $1, $4); }
    ;

opt_assign: %empty { $$ = NULL; }
    | TOK_ASSIGN expr { $$ = $2; }
    ;

type: TOK_IDENT { $$ = ref_ident(x, @$, $1); }
    | TOK_BEGIN_TEMPLATE type TOK_COLON type_list TOK_END_TEMPLATE { $$ = ref_instance(x, @$, $2, $4); }
    | TOK_MUL type { $$ = ref_pointer(x, @$, $2); }
    | TOK_BITAND type { $$ = ref_reference(x, @$, $2); }
    | TOK_OPAQUE TOK_LPAREN TOK_STRING TOK_RPAREN { $$ = ref_opaque_text(x, @$, $3); }
    | TOK_OPAQUE TOK_LPAREN TOK_IDENT TOK_RPAREN { $$ = ref_opaque(x, @$, $3); }
    | TOK_BEGIN_TEMPLATE TOK_CONST TOK_COLON type TOK_END_TEMPLATE { $$ = ref_instance(x, @$, ref_ident(x, @$, (char*)"const"), vector_init($4, BISON_ARENA(x))); }
    ;

type_list: type { $$ = vector_init($1, BISON_ARENA(x)); }
    | type_list TOK_COMMA type { vector_push(&$1, $3); $$ = $1; }
    ;

path: TOK_IDENT { $$ = vector_init($1, BISON_ARENA(x)); }
    | path TOK_COLON2 TOK_IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

opt_attrib_seq: %empty { $$ = &kEmptyVector; }
    | TOK_BEGIN_ATTRIBUTE attrib_seq TOK_END_ATTRIBUTE { $$ = $2; }
    ;

attrib_seq: attrib { $$ = vector_init($1, BISON_ARENA(x)); }
    | attrib_seq TOK_COMMA attrib { vector_push(&$1, $3); $$ = $1; }
    ;

attrib: TOK_DEPRECATED TOK_LPAREN string_list TOK_RPAREN { $$ = ref_attrib_deprecated(x, @$, $3); }
    | TOK_TYPEID TOK_LPAREN expr TOK_RPAREN { $$ = ref_attrib_typeid(x, @$, $3); }
    | TOK_LAYOUT TOK_LPAREN layout_attrib TOK_RPAREN { $$ = ref_attrib_tag(x, @$, $3); }
    | TOK_ALIGNAS TOK_LPAREN expr TOK_RPAREN { $$ = ref_attrib_alignas(x, @$, $3); }
    | TOK_CXXNAME TOK_LPAREN TOK_IDENT TOK_RPAREN { $$ = ref_attrib_cxxname(x, @$, $3); }
    | TOK_REMOTE { $$ = ref_attrib_remote(x, @$); }
    | simple_attrib { $$ = ref_attrib_tag(x, @$, $1); }
    ;

simple_attrib: TOK_TRANSIENT { $$ = eAttribTransient; }
    | TOK_BITFLAGS { $$ = eAttribBitflags; }
    | TOK_ARITHMATIC { $$ = eAttribArithmatic; }
    | TOK_ITERATOR { $$ = eAttribIterator; }
    | TOK_ORDERED { $$ = eAttribOrdered; }
    | TOK_INTERNAL { $$ = eAttribInternal; }
    | TOK_FACADE { $$ = eAttribFacade; }
    | TOK_EXTERNAL { $$ = eAttribExternal; }
    ;

layout_attrib: TOK_SYSTEM { $$ = eAttribLayoutSystem; }
    | TOK_EXPORT { $$ = eAttribLayoutStable; }
    | TOK_CBUFFER { $$ = eAttribLayoutCBuffer; }
    | TOK_PACKED { $$ = eAttribLayoutPacked; }
    | TOK_DEFAULT { $$ = eAttribLayoutAny; }
    ;

string_list: TOK_STRING { $$ = stringlist_begin(x, @$, $1); }
    | string_list TOK_STRING { $$ = stringlist_append($1, $2); }
    ;

primary_expression: TOK_IDENT { $$ = ref_ident(x, @$, $1); }
    | string_list { $$ = ref_string(x, @$, $1); }
    | TOK_LPAREN expr TOK_RPAREN { $$ = $2; }
    | TOK_INTEGER { $$ = ref_integer(x, @$, $1); }
    | TOK_TRUE { $$ = ref_bool(x, @$, true); }
    | TOK_FALSE { $$ = ref_bool(x, @$, false); }
    ;

postfix_expression: primary_expression { $$ = $1; }
    ;

unary_expression: postfix_expression { $$ = $1; }
    | TOK_NOT unary_expression { $$ = ref_unary(x, @$, eUnaryNot, $2); }
    | TOK_MINUS unary_expression { $$ = ref_unary(x, @$, eUnaryNeg, $2); }
    ;

multiplicative_expression: unary_expression { $$ = $1; }
    | multiplicative_expression TOK_MUL unary_expression { $$ = ref_binary(x, @$, eBinaryMul, $1, $3); }
    | multiplicative_expression TOK_DIV unary_expression { $$ = ref_binary(x, @$, eBinaryDiv, $1, $3); }
    | multiplicative_expression TOK_MOD unary_expression { $$ = ref_binary(x, @$, eBinaryRem, $1, $3); }
    ;

additive_expression: multiplicative_expression { $$ = $1; }
    | additive_expression TOK_PLUS multiplicative_expression { $$ = ref_binary(x, @$, eBinaryAdd, $1, $3); }
    | additive_expression TOK_MINUS multiplicative_expression { $$ = ref_binary(x, @$, eBinarySub, $1, $3); }
    ;

shift_expression: additive_expression { $$ = $1; }
    | shift_expression TOK_LSHIFT additive_expression { $$ = ref_binary(x, @$, eBinaryShl, $1, $3); }
    | shift_expression TOK_RSHIFT additive_expression { $$ = ref_binary(x, @$, eBinaryShr, $1, $3); }
    ;

relational_expression: shift_expression { $$ = $1; }
    | relational_expression TOK_LT shift_expression { $$ = ref_compare(x, @$, eCompareLt, $1, $3); }
    | relational_expression TOK_GT shift_expression { $$ = ref_compare(x, @$, eCompareGt, $1, $3); }
    | relational_expression TOK_LTE shift_expression { $$ = ref_compare(x, @$, eCompareLte, $1, $3); }
    | relational_expression TOK_GTE shift_expression { $$ = ref_compare(x, @$, eCompareGte, $1, $3); }
    ;

equality_expression: relational_expression { $$ = $1; }
    | equality_expression TOK_EQ relational_expression { $$ = ref_compare(x, @$, eCompareEq, $1, $3); }
    | equality_expression TOK_NEQ relational_expression { $$ = ref_compare(x, @$, eCompareNeq, $1, $3); }
    ;

and_expression: equality_expression { $$ = $1; }
    | and_expression TOK_BITAND equality_expression { $$ = ref_binary(x, @$, eBinaryBitAnd, $1, $3); }
    ;

exclusive_or_expression: and_expression { $$ = $1; }
    | exclusive_or_expression TOK_XOR and_expression { $$ = ref_binary(x, @$, eBinaryXor, $1, $3); }
    ;

inclusive_or_expression: exclusive_or_expression { $$ = $1; }
    | inclusive_or_expression TOK_BITOR exclusive_or_expression { $$ = ref_binary(x, @$, eBinaryBitOr, $1, $3); }
    ;

logical_and_expression: inclusive_or_expression { $$ = $1; }
    | logical_and_expression TOK_AND inclusive_or_expression { $$ = ref_compare(x, @$, eCompareAnd, $1, $3); }
    ;

logical_or_expression: logical_and_expression { $$ = $1; }
    | logical_or_expression TOK_OR logical_and_expression { $$ = ref_compare(x, @$, eCompareOr, $1, $3); }
    ;

    /* expression */
expr: logical_or_expression { $$ = $1; }
    ;

%%
