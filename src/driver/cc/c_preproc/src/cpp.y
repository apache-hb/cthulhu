%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cpp}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "memory/memory.h"
    #include "base/log.h"
    #include "cpp/scan.h"
    #define YYSTYPE CPPSTYPE
    #define YYLTYPE CPPLTYPE
}

%{
int cpplex(void *yylval, void *yylloc, void *yyscanner);
void cpperror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *text;
    text_t text2;
    cpp_number_t number;

    vector_t *vector;
    cpp_params_t params;

    cpp_ast_t *ast;
}

%token<text>
    // produced when inside a directive
    TOK_PP_IDENT "identifier (preprocessor)"
    // produced when a macro has arguments
    TOK_PP_NAME_MACRO "name of a macro (preprocessor)"
    // double quote string literal appearing in a directive
    TOK_PP_STRING "string literal (preprocessor)"
    // #id inside a define directive
    TOK_PP_STRINGIFY "macro stringification (preprocessor)"

    TOK_PP_PASTE "token paste"
    // <> include file
    TOK_SYSTEM "system header"

%token<text2>
    // double quote string literals
    TOK_STRING "string literal"
    // produced when outside a directive
    TOK_IDENT "identifier"
    TOK_WHITESPACE "whitespace"
    TOK_TEXT "text"

%token<number>
    TOK_NUMBER "number"

%token
    TOK_DIRECTIVE "#"
    TOK_INCLUDE "include"
    TOK_DEFINE "define"
    TOK_UNDEF "undef"
    TOK_IF "if"
    TOK_IFDEF "ifdef"
    TOK_IFNDEF "ifndef"
    TOK_ELSE "else"
    TOK_ELIF "elif"
    TOK_ENDIF "endif"
    TOK_LINE "line"
    TOK_PRAGMA "pragma"
    TOK_ERROR "error"
    TOK_WARNING "warning"

    TOK_LPAREN "`(`"
    TOK_RPAREN "`)`"
    TOK_COMMA "`,`"

    TOK_PP_CONCAT "`##` (concat)"

    TOK_PP_LPAREN "`(` (directive)"
    TOK_PP_RPAREN "`)` (directive)"
    TOK_PP_COMMA "`,` (directive)"
    TOK_PP_ELLIPSIS "`...` (directive)"
    TOK_PP_NOT "`!` (directive)"
    TOK_PP_AND "`&&` (directive)"
    TOK_PP_OR "`||` (directive)"
    TOK_PP_EQ "`==` (directive)"
    TOK_PP_NE "`!=` (directive)"
    TOK_PP_LT "`<` (directive)"
    TOK_PP_LE "`<=` (directive)"
    TOK_PP_GT "`>` (directive)"
    TOK_PP_GE "`>=` (directive)"
    TOK_PP_PLUS "`+` (directive)"
    TOK_PP_MINUS "`-` (directive)"
    TOK_PP_MUL "`*` (directive)"
    TOK_PP_DIV "`/` (directive)"
    TOK_PP_MOD "`%` (directive)"
    TOK_PP_XOR "`^` (directive)"
    TOK_PP_BITNOT "`~` (directive)"
    TOK_PP_SHL "`<<` (directive)"
    TOK_PP_SHR "`>>` (directive)"
    TOK_PP_BITAND "`&` (directive)"
    TOK_PP_BITOR "`|` (directive)"
    TOK_PP_QUESTION "`?` (directive)"
    TOK_PP_COLON "`:` (directive)"
    TOK_PP_DEFINED "`defined` (directive)"

%type<vector>
    define_opt_body
    directive_body
    define_params
    macro_args

%type<params>
    define_opt_params

%type<ast>
    directive_token

%type<text>
    define_param
    macro_arg

%start entry

%%

entry: %empty
    | entry any
    ;

any: TOK_DIRECTIVE directive
    | TOK_IDENT { cpp_expand_ident(x, @$, $1.text, $1.size); }
    | TOK_IDENT TOK_LPAREN macro_args TOK_RPAREN { cpp_expand_macro(x, @$, $1.text, $1.size, $3); }
    | TOK_STRING { cpp_scan_consume(x, $1.text, $1.size, false); }
    //| TOK_LPAREN { cpp_scan_consume(x, "(", 1, false); }
    //| TOK_RPAREN { cpp_scan_consume(x, ")", 1, false); }
    | TOK_COMMA { cpp_scan_consume(x, ",", 1, false); }
    | TOK_WHITESPACE { cpp_scan_consume(x, $1.text, $1.size, false); }
    | TOK_TEXT { cpp_expand_ident(x, @$, $1.text, $1.size); }
    | error
    ;

macro_args: %empty { $$ = vector_of(0); }
    | macro_args TOK_COMMA macro_arg { vector_push(&$1, $3); $$ = $1; }
    | macro_arg { $$ = vector_init($1); }
    ;

macro_arg: TOK_PP_STRING { $$ = $1; }
    | TOK_PP_IDENT { $$ = $1; }
    | TOK_TEXT { $$ = $1.text; }
    | TOK_NUMBER { $$ = $1.text; }
    | TOK_IDENT { $$ = $1.text; }
    ;

directive: directive_include
    | directive_define
    | directive_undef
    | directive_if
    | directive_ifdef
    | directive_ifndef
    | directive_else
    | directive_elif
    | directive_endif
    | directive_line
    | directive_pragma
    | directive_error
    | directive_warning
    | TOK_PP_IDENT
    ;

directive_include: TOK_INCLUDE TOK_PP_STRING { cpp_accept_include(scan, $2); }
    | TOK_INCLUDE TOK_SYSTEM { cpp_accept_include(scan, $2); }
    | TOK_INCLUDE TOK_PP_IDENT { cpp_accept_define_include(scan, $2); }
    ;

directive_define: TOK_DEFINE TOK_PP_IDENT define_opt_body { cpp_add_define(x, @$, $2, $3); }
    | TOK_DEFINE TOK_PP_NAME_MACRO define_opt_params TOK_PP_RPAREN define_opt_body { cpp_add_macro(x, @$, $2, $3, $5); }
    ;

define_opt_params: %empty { $$ = make_params(vector_of(0), false); }
    | define_params { $$ = make_params($1, false); }
    | define_params TOK_PP_COMMA TOK_PP_ELLIPSIS { $$ = make_params($1, true); }
    | TOK_PP_ELLIPSIS { $$ = make_params(vector_of(0), true); }
    ;

define_params: define_param { $$ = vector_init($1); }
    | define_params TOK_PP_COMMA define_param { vector_push(&$1, $3); $$ = $1; }
    ;

define_param: TOK_PP_IDENT { $$ = $1; }
    | TOK_ERROR { $$ = ctu_strdup("error"); }
    | TOK_WARNING { $$ = ctu_strdup("warning"); }
    ;

define_opt_body: directive_body { $$ = $1; }
    | %empty { $$ = vector_of(0); }
    ;

directive_undef: TOK_UNDEF TOK_PP_IDENT { cpp_remove_define(x, @$, $2); }
    ;

directive_if: TOK_IF directive_body { enter_branch(x, @$, $2); }
    ;

directive_ifdef: TOK_IFDEF TOK_PP_IDENT { enter_ifdef(x, @$, $2); }
    ;

directive_ifndef: TOK_IFNDEF TOK_PP_IDENT { enter_ifndef(x, @$, $2); }
    ;

directive_else: TOK_ELSE { else_branch(x, @$); }
    ;

directive_elif: TOK_ELIF directive_body { elif_branch(x, @$, $2); }
    ;

directive_endif: TOK_ENDIF { leave_branch(x, @$); }
    ;

directive_line: TOK_LINE message_body
    ;

directive_pragma: TOK_PRAGMA directive_body { cpp_accept_pragma(x, @$, $2); }
    ;

directive_error: TOK_ERROR message_body
    ;

directive_warning: TOK_WARNING message_body
    ;

directive_body: directive_token { $$ = vector_init($1); }
    | directive_body directive_token { vector_push(&$1, $2); $$ = $1; }
    ;

directive_token: TOK_PP_STRING { $$ = cpp_string(x, @$, $1); }
    | TOK_PP_IDENT  { $$ = cpp_ident(x, @$, $1); }
    | TOK_PP_LPAREN { $$ = cpp_lparen(x, @$); }
    | TOK_PP_RPAREN { $$ = cpp_rparen(x, @$); }
    | TOK_PP_COMMA  { $$ = cpp_comma(x, @$); }
    | TOK_PP_ELLIPSIS   { $$ = cpp_paste(x, @$, "..."); }
    | TOK_PP_STRINGIFY  { $$ = cpp_stringify(x, @$, $1); }
    | TOK_PP_CONCAT { $$ = cpp_concat(x, @$); }
    | TOK_PP_PASTE  { $$ = cpp_paste(x, @$, $1); }
    | TOK_IF        { $$ = cpp_paste(x, @$, "if"); }
    | TOK_ELSE      { $$ = cpp_paste(x, @$, "else"); }
    | TOK_NUMBER    { $$ = cpp_number(x, @$, $1); }
    | TOK_PP_NOT    { $$ = cpp_unary(x, @$, eUnaryNot); }
    | TOK_PP_AND    { $$ = cpp_compare(x, @$, eCompareAnd); }
    | TOK_PP_OR     { $$ = cpp_compare(x, @$, eCompareOr); }
    | TOK_PP_EQ     { $$ = cpp_compare(x, @$, eCompareEq); }
    | TOK_PP_NE     { $$ = cpp_compare(x, @$, eCompareNeq); }
    | TOK_PP_LT     { $$ = cpp_compare(x, @$, eCompareLt); }
    | TOK_PP_LE     { $$ = cpp_compare(x, @$, eCompareLte); }
    | TOK_PP_GT     { $$ = cpp_compare(x, @$, eCompareGt); }
    | TOK_PP_GE     { $$ = cpp_compare(x, @$, eCompareGte); }
    | TOK_PP_PLUS   { $$ = cpp_binary(x, @$, eBinaryAdd); }
    | TOK_PP_MINUS  { $$ = cpp_binary(x, @$, eBinarySub); }
    | TOK_PP_MUL    { $$ = cpp_binary(x, @$, eBinaryMul); }
    | TOK_PP_DIV    { $$ = cpp_binary(x, @$, eBinaryDiv); }
    | TOK_PP_MOD    { $$ = cpp_binary(x, @$, eBinaryRem); }
    | TOK_PP_XOR    { $$ = cpp_binary(x, @$, eBinaryXor); }
    | TOK_PP_BITNOT { $$ = cpp_unary(x, @$, eUnaryFlip); }
    | TOK_PP_SHL    { $$ = cpp_binary(x, @$, eBinaryShl); }
    | TOK_PP_SHR    { $$ = cpp_binary(x, @$, eBinaryShr); }
    | TOK_PP_BITAND { $$ = cpp_binary(x, @$, eBinaryBitAnd); }
    | TOK_PP_BITOR  { $$ = cpp_binary(x, @$, eBinaryBitOr); }
    | TOK_PP_QUESTION { $$ = cpp_ternary(x, @$); }
    | TOK_PP_COLON  { $$ = cpp_colon(x, @$); }
    | TOK_PP_DEFINED { $$ = cpp_defined(x, @$); }
    | TOK_WARNING   { $$ = cpp_paste(x, @$, "warning"); }
    | TOK_ERROR     { $$ = cpp_paste(x, @$, "error"); }
    | TOK_INCLUDE   { $$ = cpp_paste(x, @$, "include"); }
    ;

message_body: %empty
    | message_body message_item
    ;

message_item: TOK_WARNING
    | TOK_ERROR
    | TOK_PP_STRING
    | TOK_PP_IDENT
    | TOK_PP_LPAREN
    | TOK_PP_RPAREN
    | TOK_PP_COMMA
    | TOK_PP_ELLIPSIS
    | TOK_PP_STRINGIFY
    | TOK_PP_CONCAT
    | TOK_PP_PASTE
    | TOK_IF
    | TOK_ELSE
    | TOK_NUMBER
    | TOK_PP_NOT
    | TOK_PP_AND
    | TOK_PP_OR
    | TOK_PP_EQ
    | TOK_PP_NE
    | TOK_PP_LT
    | TOK_PP_LE
    | TOK_PP_GT
    | TOK_PP_GE
    | TOK_PP_PLUS
    | TOK_PP_MINUS
    | TOK_PP_MUL
    | TOK_PP_DIV
    | TOK_PP_MOD
    | TOK_PP_XOR
    | TOK_PP_BITNOT
    | TOK_PP_SHL
    | TOK_PP_SHR
    | TOK_PP_BITAND
    | TOK_PP_BITOR
    | TOK_PP_QUESTION
    | TOK_PP_COLON
    | TOK_PP_DEFINED
    | TOK_INCLUDE
    ;

%%
