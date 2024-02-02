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
    #include "std/map.h"
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
    map_t *map;

    ref_param_t param;
    ref_flags_t flags;
    ref_attrib_tag_t attrib;
    ref_privacy_t privacy;
    ref_pair_t pair;

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
    using_decl
    header_item
    config config_body
    config_value opaque
    union_field union_decl opt_case_init
    class_var_decl var_name_decl

%type<vector>
    path
    decl_seq
    class_body_seq
    case_seq
    opt_attrib_seq
    attrib_seq
    ident_list
    opt_tparams
    opt_params
    param_list
    module
    opt_header_seq
    header_seq
    union_field_seq var_decl_seq union_field_body

%type<typevec> string_list
%type<param> passing
%type<flags> opt_decl_flags_seq decl_flags_seq decl_flag
%type<attrib> simple_attrib layout_attrib
%type<privacy> privacy_spec
%type<pair> doc_item
%type<map> doc_body

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
    TOK_UNION "union"
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

    /* attribute only keywords */
    TOK_TRANSIENT "transient"
    TOK_DOC "doc"
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
    TOK_INTERNAL "internal"
    TOK_EXTERNAL "external"
    TOK_ORDERED "ordered"
    TOK_FORMAT "format"
    TOK_INPUT "input"
    TOK_LOOKUP "lookup"
    TOK_SERIALIZE "serialize"
    TOK_CHECKSUM "checksum"

    /* config only keywords */
    TOK_CONFIG "config"
    TOK_END_CONFIG "; (end config)"
    TOK_CFG_API "api (config option)"
    TOK_CFG_VECTOR "vector (config option)"
    TOK_CFG_SPAN "span (config option)"
    TOK_CFG_ARRAY "array (config option)"
    TOK_CFG_STRING "string (config option)"

    TOK_NULL "null"
    TOK_TRUE "true"
    TOK_FALSE "false"
    TOK_SEALED "sealed"

    TOK_CASE "case"

    TOK_IN "in"
    TOK_OUT "out"

    TOK_BEGIN_TEMPLATE "!< (begin template)"
    TOK_END_TEMPLATE "> (end template)"

%start program

%%

program: module opt_header_seq decl_seq { scan_set(x, ref_program(x, @$, $1, $2, $3)); };

module: TOK_MODULE path TOK_SEMICOLON { $$ = $2; }
    ;

opt_header_seq: %empty { $$ = &kEmptyVector; }
    | header_seq { $$ = $1; }
    ;

header_seq: header_item { $$ = vector_init($1, BISON_ARENA(x)); }
    | header_seq header_item { vector_push(&$1, $2); $$ = $1; }
    ;

header_item: config { $$ = $1; }
    | import { $$ = $1; }
    ;

import: TOK_IMPORT TOK_STRING TOK_SEMICOLON { $$ = ref_import(x, @$, $2); }
    ;

config: TOK_CONFIG config_body TOK_END_CONFIG { $$ = $2; }
    ;

config_body: TOK_CFG_API TOK_ASSIGN config_value { $$ = ref_config_tag(x, @$, eRefConfigSerialize, $3); }
    | TOK_CFG_VECTOR TOK_ASSIGN config_value { $$ = ref_config_tag(x, @$, eRefConfigVector, $3); }
    | TOK_CFG_SPAN TOK_ASSIGN config_value { $$ = ref_config_tag(x, @$, eRefConfigSpan, $3); }
    | TOK_CFG_ARRAY TOK_ASSIGN config_value { $$ = ref_config_tag(x, @$, eRefConfigArray, $3); }
    | TOK_CFG_STRING TOK_ASSIGN config_value { $$ = ref_config_tag(x, @$, eRefConfigString, $3); }
    ;

config_value: TOK_IDENT { $$ = ref_ident(x, @$, $1); }
    | opaque { $$ = $1; }
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
    | using_decl { $$ = $1; }
    | union_decl { $$ = $1; }
    ;

union_decl: TOK_UNION TOK_IDENT TOK_LPAREN var_name_decl TOK_RPAREN TOK_LBRACE union_field_seq TOK_RBRACE { $$ = ref_union(x, @$, $2, $4, $7); }
    ;

union_field_seq: union_field { $$ = vector_init($1, BISON_ARENA(x)); }
    | union_field_seq union_field { vector_push(&$1, $2); $$ = $1; }
    ;

union_field: opt_attrib_seq TOK_CASE TOK_LPAREN ident_list TOK_RPAREN union_field_body { $$ = ref_union_field(x, @$, $4, $6); ref_set_attribs($$, $1); }
    ;

union_field_body: TOK_LBRACE var_decl_seq TOK_RBRACE { $$ = $2; }
    ;

var_decl_seq: var_name_decl { $$ = vector_init($1, BISON_ARENA(x)); }
    | var_decl_seq var_name_decl { vector_push(&$1, $2); $$ = $1; }
    ;

opt_inherit: %empty { $$ = NULL; }
    | TOK_COLON type { $$ = $2; }
    ;

using_decl: TOK_ALIAS TOK_IDENT TOK_ASSIGN type { $$ = ref_using(x, @$, $2, $4); }
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

case_item_inner: TOK_CASE TOK_IDENT opt_case_init { $$ = ref_case(x, @$, $2, $3, false); }
    | TOK_DEFAULT TOK_IDENT opt_case_init { $$ = ref_case(x, @$, $2, $3, true); }
    | opt_decl_flags_seq method_decl TOK_SEMICOLON { ref_set_flags($2, $1); $$ = $2; }
    ;

opt_case_init: %empty { $$ = NULL; }
    | TOK_ASSIGN case_value { $$ = $2; }
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

var_name_decl: TOK_IDENT TOK_COLON type { $$ = ref_field(x, @$, $1, $3, NULL); }
    ;

class_var_decl: TOK_IDENT TOK_COLON type opt_assign { $$ = ref_field(x, @$, $1, $3, $4); }
    ;

class_body_inner: class_var_decl { $$ = $1; }
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
    | TOK_MUL type { $$ = ref_pointer(x, @$, $2); }
    | TOK_BITAND type { $$ = ref_reference(x, @$, $2); }
    | TOK_LSQUARE type TOK_RSQUARE { $$ = ref_vector(x, @$, $2); }
    | TOK_CONST type { $$ = ref_const(x, @$, $2); }
    | opaque { $$ = $1; }
    ;

opaque: TOK_OPAQUE TOK_LPAREN TOK_STRING TOK_RPAREN { $$ = ref_opaque_text(x, @$, $3); }
    | TOK_OPAQUE TOK_LPAREN TOK_IDENT TOK_RPAREN { $$ = ref_opaque(x, @$, $3); }
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

attrib: TOK_DEPRECATED TOK_LPAREN string_list TOK_RPAREN { $$ = ref_attrib_deprecated(x, @$, ref_make_string($3)); }
    | TOK_TYPEID TOK_LPAREN expr TOK_RPAREN { $$ = ref_attrib_typeid(x, @$, $3); }
    | TOK_LAYOUT TOK_LPAREN layout_attrib TOK_RPAREN { $$ = ref_attrib_tag(x, @$, $3); }
    | TOK_ALIGNAS TOK_LPAREN expr TOK_RPAREN { $$ = ref_attrib_alignas(x, @$, $3); }
    | TOK_CXXNAME TOK_LPAREN TOK_IDENT TOK_RPAREN { $$ = ref_attrib_string(x, @$, eAttribCxxName, $3); }
    | TOK_REMOTE { $$ = ref_attrib_remote(x, @$); }
    | TOK_FORMAT TOK_LPAREN string_list TOK_RPAREN { $$ = ref_attrib_string(x, @$, eAttribFormat, ref_make_string($3)); }
    | simple_attrib { $$ = ref_attrib_tag(x, @$, $1); }
    | TOK_DOC TOK_LPAREN doc_body TOK_RPAREN { $$ = ref_attrib_docs(x, @$, $3); }
    | TOK_SERIALIZE { $$ = ref_attrib_tag(x, @$, eAttribSerialize); }
    | TOK_CHECKSUM { $$ = ref_attrib_tag(x, @$, eAttribChecksum); }
    ;

doc_body: doc_item { $$ = map_new(32, kTypeInfoString, BISON_ARENA(x)); map_set($$, $1.ident, $1.body); }
    | doc_body doc_item { map_set($1, $2.ident, $2.body); $$ = $1; }
    ;

doc_item: TOK_IDENT TOK_ASSIGN string_list { $$ = ref_pair($1, $3); }
    ;

simple_attrib: TOK_TRANSIENT { $$ = eAttribTransient; }
    | TOK_BITFLAGS { $$ = eAttribBitflags; }
    | TOK_ARITHMATIC { $$ = eAttribArithmatic; }
    | TOK_ITERATOR { $$ = eAttribIterator; }
    | TOK_ORDERED { $$ = eAttribOrdered; }
    | TOK_INTERNAL { $$ = eAttribInternal; }
    | TOK_FACADE { $$ = eAttribFacade; }
    | TOK_EXTERNAL { $$ = eAttribExternal; }
    | TOK_LOOKUP { $$ = eAttribLookupKey; }
    ;

layout_attrib: TOK_SYSTEM { $$ = eAttribLayoutSystem; }
    | TOK_EXPORT { $$ = eAttribLayoutStable; }
    | TOK_SERIALIZE { $$ = eAttribLayoutStable; }
    | TOK_CBUFFER { $$ = eAttribLayoutConstBuffer; }
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
