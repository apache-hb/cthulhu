%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 1 /* dangling else causes 1 shift/reduce conflict */
%define api.prefix {cc}

%code top {
    // grammar is based on N3096
    #include "interop/flex.h"
    #include "interop/bison.h"

    #include "cthulhu/tree/ops.h"
    #include "base/log.h"
    #include "std/vector.h"
}

%code requires {
    #include "c/ast.h"
    #include "c/scan.h"
    #define YYSTYPE CCSTYPE
    #define YYLTYPE CCLTYPE
}

%{
int cclex(void *yylval, void *yylloc, void *yyscanner);
void ccerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;

    text_t text;

    bool boolean;

    mpz_t mpz;

    c_type_qualifier_t type_qualifier;
    c_storage_class_t storage_class;
    c_callconv_t callconv;

    vector_t *vector;

    sign_t sign;
    digit_t digit;

    c_ast_t *ast;
}

%token<ident>
    IDENT "identifier"
    TYPEDEF_NAME "typedef name"

%token<mpz>
    DIGIT "digit"

%token<text>
    STRING "string"

%nonassoc IF "if"
%nonassoc ELSE "else"

%token
    TYPEDEF "typedef"

    VOID "void"
    BOOL "_Bool"

    CHAR "char"
    SHORT "short"
    INT "int"
    LONG "long"

    FLOAT "float"
    DOUBLE "double"

    SIGNED "signed"
    UNSIGNED "unsigned"

    ATOMIC "_Atomic"
    CONST "const"
    VOLATILE "volatile"
    RESTRICT "restrict"
    AUTO "auto"
    EXTERN "extern"
    INLINE "inline"
    STATIC "static"
    REGISTER "register"
    CONSTEXPR "constexpr"

    BREAK "break"
    CASE "case"
    CONTINUE "continue"
    DEFAULT "default"

    DO "do"
    WHILE "while"
    FOR "for"
    GOTO "goto"
    RETURN "return"
    SWITCH "switch"

    SIZEOF "sizeof"
    ALIGNAS "_Alignas"
    ALIGNOF "_Alignof"

    COMPLEX "_Complex"
    GENERIC "_Generic"
    IMAGINARY "_Imaginary"

    NORETURN "_Noreturn"

    STATIC_ASSERT "_Static_assert"
    THREAD_LOCAL "_Thread_local"

    STRUCT "struct"
    UNION "union"
    ENUM "enum"

    SEMICOLON ";"

    COLON ":"
    COLON2 "::"

    COMMA ","
    DOT3 "..."
    DOT "."

    INC "++"
    DEC "--"

    ARROW "->"
    NOT "!"

    ADDEQ "+="
    ADD "+"
    SUBEQ "-="
    SUB "-"
    MULEQ "*="
    MUL "*"
    DIVEQ "/="
    DIV "/"
    MODEQ "%="
    MOD "%"

    SHL "<<"
    SHLEQ "<<="
    SHR ">>"
    SHREQ ">>="

    LTE "<="
    LT "<"
    GTE ">="
    GT ">"

    AND "&&"
    OR "||"

    EQ "=="
    NEQ "!="
    BITNOT "~"

    ANDEQ "&="
    BITAND "&"

    OREQ "|="
    BITOR "|"

    ASSIGN "="
    QUESTION "?"
    BITXOR "^"
    XOREQ "^="
    HASH "#"

    LBRACE "{"
    RBRACE "}"

    LBRACKET "["
    RBRACKET "]"

    LPAREN "("
    RPAREN ")"

    MODULE "_Module"
    IMPORT "_Import"
    EXPORT "_Export"
    PRIVATE "_Private"

    STDCALL "__stdcall"
    CDECL "__cdecl"
    FASTCALL "__fastcall"
    THISCALL "__thiscall"
    VECTORCALL "__vectorcall"
    ATTRIBUTE "__attribute__"
    DECLSPEC "__declspec"

%type<vector>
    block_item_list
    translation_unit
    module_path
    declaration_specifiers
    init_declarator_list
    attribute_specifier_seq
    specifier_qualifier_list
    expression
    balanced_token_seq
    attribute_argument_clause

%type<storage_class>
    storage_class_specifier

%type<callconv>
    ext_attribute_callconv

%type<type_qualifier>
    type_qualifier

%type<ast>
    block_item
    label
    declaration
    unlabelled_statement
    expression_statement
    primary_block
    jump_statement
    constant_expression
    selection_statement
    iteration_statement
    module_declaration
    function_definition
    external_declaration
    init_declarator
    initializer
    declarator
    declaration_specifier
    attribute_specifier
    type_specifier_qualifier
    alignment_specifier
    type_name
    opt_expression
    string_list
    cast_expression
    assignment_expression
    conditional_expression
    type_specifier
    balanced_token

%type<boolean>
    opt_export

%start start

%%

start: translation_unit { scan_set(x, $1); }
    ;

translation_unit: external_declaration { $$ = vector_init($1, BISON_ARENA(x)); }
    | translation_unit external_declaration { vector_push(&$1, $2); $$ = $1; }
    ;

opt_export: %empty { $$ = false; }
    | EXPORT { $$ = true; }
    ;

module_declaration: MODULE { $$ = c_ast_module_private_fragment(x, @$); }
    | MODULE module_path { $$ = c_ast_module_public_fragment(x, @$, $2); }
    | MODULE COLON PRIVATE { $$ = c_ast_module_private_fragment(x, @$); }
    | IMPORT module_path { $$ = c_ast_module_import(x, @$, $2); }
    ;

external_declaration: opt_export function_definition { $$ = $2; }
    | opt_export declaration { $$ = NULL; }
    | opt_export module_declaration SEMICOLON {
        c_ast_apply_export($2, $1);
        $$ = $2;
    }
    | directive {
        ctu_log("directive");
        $$ = NULL;
    }
    | STATIC_ASSERT LPAREN constant_expression COMMA string_list RPAREN SEMICOLON {
        ctu_log("static assert");
        $$ = NULL;
    }
    | error {
        cc_on_error(x, "error", @$);
        $$ = NULL;
    }
    ;

function_definition: declaration_specifiers declarator function_body { $$ = NULL; }
    ;

function_body: compound_statement
    ;

compound_statement: LBRACE RBRACE
    | LBRACE block_item_list RBRACE
    ;

block_item_list: block_item { $$ = vector_init($1, BISON_ARENA(x)); }
    | block_item_list block_item { vector_push(&$1, $2); $$ = $1; }
    ;

block_item: declaration
    | unlabelled_statement
    ;

unlabelled_statement: expression_statement
    | primary_block
    | jump_statement
    ;

labeled_statement: label statement
    ;

label: IDENT COLON { $$ = c_ast_label(x, @$, $1); }
    | CASE constant_expression COLON { $$ = c_ast_case(x, @$, $2); }
    | DEFAULT COLON { $$ = c_ast_default(x, @$); }
    ;

statement: unlabelled_statement
    | labeled_statement
    ;

primary_block: compound_statement { $$ = NULL; }
    | selection_statement { $$ = NULL; }
    | iteration_statement { $$ = NULL; }
    ;

selection_statement: IF LPAREN expression RPAREN secondary_block { $$ = NULL; }
    | IF LPAREN expression RPAREN secondary_block ELSE secondary_block { $$ = NULL; }
    | SWITCH LPAREN expression RPAREN secondary_block { $$ = NULL; }
    ;

opt_expression: %empty { $$ = NULL; }
    | expression { $$ = c_ast_expand_exprs(x, @$, $1); }
    ;

iteration_statement: WHILE LPAREN expression RPAREN secondary_block { $$ = NULL; }
    | DO secondary_block WHILE LPAREN expression RPAREN SEMICOLON { $$ = NULL; }
    | FOR LPAREN opt_expression SEMICOLON opt_expression SEMICOLON opt_expression RPAREN secondary_block { $$ = NULL; }
    | FOR LPAREN declaration opt_expression SEMICOLON opt_expression RPAREN secondary_block { $$ = NULL; }
    ;

jump_statement: GOTO IDENT SEMICOLON { $$ = c_ast_goto(x, @$, $2); }
    | CONTINUE SEMICOLON { $$ = c_ast_continue(x, @$); }
    | BREAK SEMICOLON { $$ = c_ast_break(x, @$); }
    | RETURN opt_expression SEMICOLON { $$ = c_ast_return(x, @$, $2); }
    ;

secondary_block: statement
    ;

    /* expressions */
expression_list: assignment_expression
    | expression_list COMMA assignment_expression
    ;

opt_expression_list: %empty
    | expression_list
    ;

primary_expression: IDENT
    | DIGIT
    | string_list
    | LPAREN expression RPAREN
    ;

string_list: STRING { $$ = c_ast_string(x, @$, $1); }
    | string_list STRING { $$ = c_ast_append_string(x, @$, $1, $2); }
    ;

postfix_expression: primary_expression
    | postfix_expression LBRACKET expression RBRACKET
    | postfix_expression LPAREN opt_expression_list RPAREN
    | postfix_expression DOT IDENT
    | postfix_expression ARROW IDENT
    | postfix_expression INC
    | postfix_expression DEC
    ;

unary_expression: postfix_expression
    | INC unary_expression
    | DEC unary_expression
    | unary_operator cast_expression
    | SIZEOF unary_expression
    | SIZEOF LPAREN type_name RPAREN
    | ALIGNOF LPAREN type_name RPAREN
    ;

unary_operator: AND | MUL | ADD | SUB | NOT | BITNOT
    ;

cast_expression: unary_expression { $$ = NULL; }
    | LPAREN type_name RPAREN cast_expression { $$ = c_ast_cast(x, @$, $4, $2); }
    ;

multiplicative_expression: cast_expression
    | multiplicative_expression MUL cast_expression
    | multiplicative_expression DIV cast_expression
    | multiplicative_expression MOD cast_expression
    ;

additive_expression: multiplicative_expression
    | additive_expression ADD multiplicative_expression
    | additive_expression SUB multiplicative_expression
    ;

shift_expression: additive_expression
    | shift_expression SHL additive_expression
    | shift_expression SHR additive_expression
    ;

relational_expression: shift_expression
    | relational_expression LT shift_expression
    | relational_expression GT shift_expression
    | relational_expression LTE shift_expression
    | relational_expression GTE shift_expression
    ;

equality_expression: relational_expression
    | equality_expression EQ relational_expression
    | equality_expression NEQ relational_expression
    ;

and_expression: equality_expression
    | and_expression BITAND equality_expression
    ;

exclusive_or_expression: and_expression
    | exclusive_or_expression BITXOR and_expression
    ;

inclusive_or_expression: exclusive_or_expression
    | inclusive_or_expression BITOR exclusive_or_expression
    ;

logical_and_expression: inclusive_or_expression
    | logical_and_expression AND inclusive_or_expression
    ;

logical_or_expression: logical_and_expression
    | logical_or_expression OR logical_and_expression
    ;

conditional_expression: logical_or_expression { $$ = NULL; }
    | logical_or_expression QUESTION expression COLON conditional_expression { $$ = NULL; }
    ;

assignment_expression: conditional_expression { $$ = NULL; }
    | unary_expression assignment_operator assignment_expression { $$ = NULL; }
    ;

assignment_operator: ASSIGN
    | ADDEQ | SUBEQ | MULEQ | DIVEQ | MODEQ
    | SHLEQ | SHREQ | ANDEQ | OREQ | XOREQ
    ;

expression: assignment_expression { $$ = vector_init($1, BISON_ARENA(x)); }
    | expression COMMA assignment_expression { vector_push(&$1, $3); $$ = $1; }
    ;

constant_expression: conditional_expression { $$ = $1; }
    ;

expression_statement: SEMICOLON { $$ = NULL; }
    | expression SEMICOLON { $$ = c_ast_expand_exprs(x, @$, $1); }
    ;

module_path: IDENT { $$ = vector_init($1, BISON_ARENA(x)); }
    | module_path DOT IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

    /* this is for parsing out all the pragmas and line directives */
directive: HASH directive_body ;
directive_body: %empty | directive_body directive_item ;
directive_item: IDENT | STRING | DIGIT ;

    /* declarations */
declaration: declaration_specifiers init_declarator_list SEMICOLON {
        $$ = c_ast_declarator_list(x, @$, $1, $2);
    }
    | attribute_specifier_seq declaration_specifiers init_declarator_list SEMICOLON {
        c_ast_t *decl = c_ast_declarator_list(x, @$, $2, $3);
        c_ast_apply_attributes(decl, $1);
        $$ = decl;
    }
    ;

init_declarator_list: init_declarator { $$ = vector_init($1, BISON_ARENA(x)); }
    | init_declarator_list COMMA init_declarator { vector_push(&$1, $3); $$ = $1; }
    ;

init_declarator: declarator { $$ = $1; }
    | declarator ASSIGN initializer { $$ = c_ast_init_declarator(x, @$, $1, $3); }
    ;

initializer: constant_expression { $$ = $1; }
    ;

declarator: opt_pointer direct_declarator { $$ = NULL; }
    | attribute_specifier_seq opt_pointer direct_declarator { $$ = NULL; }
    ;

opt_pointer: %empty
    | pointer
    ;

pointer: MUL opt_type_qualifier_list
    | MUL opt_type_qualifier_list pointer
    ;

opt_type_qualifier_list: %empty
    | type_qualifier_list
    ;

type_qualifier_list: type_qualifier
    | type_qualifier_list type_qualifier
    ;

direct_declarator: IDENT
    | LPAREN declarator RPAREN
    | function_declarator
    ;

function_declarator: direct_declarator LPAREN parameter_type_list RPAREN
    | direct_declarator LPAREN RPAREN
    ;

parameter_type_list: parameter_list
    | parameter_list COMMA DOT3
    ;

parameter_list: parameter_declaration
    | parameter_list COMMA parameter_declaration
    ;

parameter_declaration: declaration_specifiers declarator
    ;

declaration_specifiers: declaration_specifier { $$ = vector_init($1, BISON_ARENA(x)); }
    | declaration_specifiers declaration_specifier { vector_push(&$1, $2); $$ = $1; }
    ;

declaration_specifier: storage_class_specifier { $$ = c_ast_storage_class(x, @$, $1); }
    | type_specifier_qualifier { $$ = $1; }
    ;

storage_class_specifier: AUTO { $$ = eStorageClassAuto; }
    | CONSTEXPR { $$ = eStorageClassConstexpr; }
    | EXTERN { $$ = eStorageClassExtern; }
    | INLINE { $$ = eStorageClassInline; }
    | REGISTER { $$ = eStorageClassRegister; }
    | STATIC { $$ = eStorageClassStatic; }
    | THREAD_LOCAL { $$ = eStorageClassThreadLocal; }
    | TYPEDEF { $$ = eStorageClassTypedef; }
    ;

type_specifier_qualifier: type_specifier { $$ = NULL; }
    | type_qualifier { $$ = c_ast_type_qualifier(x, @$, $1); }
    | alignment_specifier { $$ = $1; }
    ;

    /* attributes */
attribute_specifier_seq: attribute_specifier { $$ = vector_init($1, BISON_ARENA(x)); }
    | attribute_specifier_seq attribute_specifier { vector_push(&$1, $2); $$ = $1; }
    ;

attribute_specifier: LBRACKET LBRACKET attribute_list RBRACKET RBRACKET { $$ = NULL; }
    | ATTRIBUTE LPAREN LPAREN attribute_list RPAREN RPAREN { $$ = NULL; }
    | DECLSPEC LPAREN attribute_list RPAREN { $$ = NULL; }
    | ext_attribute_callconv { $$ = c_ast_attribute_callconv(x, @$, $1); }
    | NORETURN { $$ = c_ast_attribute_noreturn(x, @$); }
    ;

ext_attribute_callconv: STDCALL { $$ = eCallStdcall; }
    | CDECL { $$ = eCallCdecl; }
    | FASTCALL { $$ = eCallFastcall; }
    | THISCALL { $$ = eCallThiscall; }
    | VECTORCALL { $$ = eCallVectorcall; }
    ;

attribute_list: attribute
    | attribute_list COMMA attribute
    ;

attribute: attribute_token
    | attribute_token attribute_argument_clause
    ;

attribute_token: IDENT
    | IDENT COLON2 IDENT
    ;

attribute_argument_clause: LPAREN balanced_token_seq RPAREN { $$ = $2; }
    ;

balanced_token_seq: balanced_token { $$ = vector_init($1, BISON_ARENA(x)); }
    | balanced_token_seq balanced_token { vector_push(&$1, $2); $$ = $1; }
    ;

balanced_token: LPAREN balanced_token_seq RPAREN { $$ = NULL; }
    | LBRACKET balanced_token_seq RBRACKET { $$ = NULL; }
    | LBRACE balanced_token_seq RBRACE { $$ = NULL; }
    | IDENT { $$ = NULL; }
    | STRING { $$ = NULL; }
    | DIGIT { $$ = NULL; }
    ;

    /* types */
type_name: specifier_qualifier_list { $$ = c_ast_type(x, @$, $1); }
    ;

specifier_qualifier_list: type_specifier_qualifier { $$ = vector_init($1, BISON_ARENA(x)); }
    | specifier_qualifier_list type_specifier_qualifier { vector_push(&$1, $2); $$ = $1; }
    ;

type_specifier: VOID { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierVoid); }
    | CHAR { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierChar); }
    | SHORT { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierShort); }
    | INT { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierInt); }
    | LONG { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierLong); }
    | FLOAT { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierFloat); }
    | DOUBLE { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierDouble); }
    | SIGNED { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierSigned); }
    | UNSIGNED { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierUnsigned); }
    | BOOL { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierBool); }
    | COMPLEX { $$ = c_ast_type_specifier(x, @$, eTypeSpecifierComplex); }
    | TYPEDEF_NAME { $$ = c_ast_typedef_name(x, @$, $1); }
    ;

type_qualifier: CONST { $$ = eTypeQualifierConst; }
    | RESTRICT { $$ = eTypeQualifierRestrict; }
    | VOLATILE { $$ = eTypeQualifierVolatile; }
    | ATOMIC { $$ = eTypeQualifierAtomic; }
    ;

alignment_specifier: ALIGNAS LPAREN type_name RPAREN { $$ = c_ast_alignas(x, @$, $3); }
    | ALIGNAS LPAREN constant_expression RPAREN { $$ = c_ast_alignas(x, @$, $3); }
    ;

%%
