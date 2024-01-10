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

    mpz_t mpz;

    vector_t *vector;

    sign_t sign;
    digit_t digit;

    c_ast_t *ast;
}

%token<ident>
    IDENT "identifier"

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

    NRETURN "_Noreturn"

    STATIC_ASSERT "_Static_assert"
    THREAD_LOCAL "_Thread_local"

    STRUCT "struct"
    UNION "union"
    ENUM "enum"

    SEMICOLON ";"

    COLON2 "::"
    COLON ":"

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

%type<vector>
    block_item_list

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

%start translation_unit

%%

translation_unit: external_declaration
    | translation_unit external_declaration
    ;

opt_export: %empty
    | EXPORT
    ;

module_declaration: MODULE
    | MODULE module_path
    | MODULE COLON PRIVATE
    | IMPORT module_path
    ;

external_declaration: opt_export function_definition
    | opt_export declaration
    | opt_export module_declaration SEMICOLON
    | directive { ctu_log("directive"); }
    | error { cc_on_error(x, "invalid declaration", @$); }
    ;

function_definition: declaration_specifiers declarator function_body
    ;

function_body: compound_statement
    ;

compound_statement: LBRACE RBRACE
    | LBRACE block_item_list RBRACE
    ;

block_item_list: block_item { $$ = vector_init($1); }
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

opt_expression: %empty
    | expression
    ;

iteration_statement: WHILE LPAREN expression RPAREN secondary_block { $$ = NULL; }
    | DO secondary_block WHILE LPAREN expression RPAREN SEMICOLON { $$ = NULL; }
    | FOR LPAREN opt_expression SEMICOLON opt_expression SEMICOLON opt_expression RPAREN secondary_block { $$ = NULL; }
    | FOR LPAREN declaration opt_expression SEMICOLON opt_expression RPAREN secondary_block { $$ = NULL; }
    ;

jump_statement: GOTO IDENT SEMICOLON { $$ = NULL; }
    | CONTINUE SEMICOLON { $$ = NULL; }
    | BREAK SEMICOLON { $$ = NULL; }
    | RETURN opt_expression SEMICOLON { $$ = NULL; }
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
    | STRING
    | LPAREN expression RPAREN
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

cast_expression: unary_expression
    | LPAREN type_name RPAREN cast_expression
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

conditional_expression: logical_or_expression
    | logical_or_expression QUESTION expression COLON conditional_expression
    ;

assignment_expression: conditional_expression
    | unary_expression assignment_operator assignment_expression
    ;

assignment_operator: ASSIGN
    | ADDEQ | SUBEQ | MULEQ | DIVEQ | MODEQ
    | SHLEQ | SHREQ | ANDEQ | OREQ | XOREQ
    ;

expression: assignment_expression
    | expression COMMA assignment_expression
    ;

constant_expression: conditional_expression { $$ = NULL; }
    ;

expression_statement: SEMICOLON { $$ = NULL; }
    | expression SEMICOLON { $$ = NULL; }
    ;

module_path: IDENT
    | module_path DOT IDENT
    ;

    /* this is for parsing out all the pragmas and line directives */
directive: HASH directive_body ;
directive_body: %empty | directive_body directive_item ;
directive_item: IDENT | STRING | DIGIT ;

    /* declarations */
declaration: declaration_specifiers init_declarator_list SEMICOLON { $$ = NULL; }
    ;

init_declarator_list: init_declarator
    | init_declarator_list COMMA init_declarator
    ;

init_declarator: declarator
    | declarator ASSIGN initializer
    ;

initializer: constant_expression
    ;

    /* for now we dont handle pointer 6.7.6 */
declarator: direct_declarator
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

declaration_specifiers: declaration_specifier
    | declaration_specifiers declaration_specifier
    ;

declaration_specifier: storage_class_specifier
    | type_specifier_qualifier
    ;

storage_class_specifier: AUTO
    | CONSTEXPR
    | EXTERN
    | INLINE
    | REGISTER
    | STATIC
    | THREAD_LOCAL
    | TYPEDEF
    ;

type_specifier_qualifier: type_specifier
    | type_qualifier
    | alignment_specifier
    ;

    /* types */
type_name: specifier_qualifier_list
    ;

specifier_qualifier_list: type_specifier_qualifier
    | specifier_qualifier_list type_specifier_qualifier
    ;

type_specifier: VOID
    | CHAR
    | SHORT
    | INT
    | LONG
    | FLOAT
    | DOUBLE
    | SIGNED
    | UNSIGNED
    | BOOL
    | COMPLEX
    ;

type_qualifier: CONST
    | RESTRICT
    | VOLATILE
    | ATOMIC
    ;

alignment_specifier: ALIGNAS LPAREN type_name RPAREN
    | ALIGNAS LPAREN constant_expression RPAREN
    ;

%%
