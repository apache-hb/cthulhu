%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {ctu}

%code requires {
    #define YYSTYPE CTUSTYPE
    #define YYLTYPE CTULTYPE
    
    #include "scan.h"
    #include "ast.h"
}

%{
#include "scan.h"
int ctulex();
void ctuerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    bool boolean;
    char *ident;
    mpz_t mpz;

    ast_t *ast;
    vector_t *vector;
}

%token<ident>
    IDENT "identifier"

%token<mpz>
    INTEGER "integer literal"

%token<boolean>
    BOOLEAN "boolean literal"


%token
    AT "`@`"
    COLON "`:`"
    COLON2 "`::`"
    SEMICOLON "`;`"
    COMMA "`,`"
    DOT "`.`"
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

    DEF "`def`"
    VAR "`var`"
    FINAL "`final`"

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

    LAMBDA "`lambda`"
    AS "`as`"

    NIL "`null`"

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

    IN "`in`"
    OUT "`out`"
    INOUT "`inout`"

    PRIVATE "`private`"
    PUBLIC "`public`"
    PROTECTED "`protected`"

    ASYNC "`async`"
    AWAIT "`await`"
    YIELD "`yield`"

    CONTRACT "`contract`"
    REQUIRES "`requires`"
    ASSERT "`assert`"
    ENSURES "`ensures`"
    INVARIANT "`invariant`"

    SIZEOF "`sizeof`"
    ALIGNOF "`alignof`"
    OFFSETOF "`offsetof`"
    TYPEOF "`typeof`"
    UUIDOF "`uuidof`"

%type<ast>
    primary postfix unary expr
    or and xor bits shift equality compare add mul

    type

%type<vector>
    path

%start program

%%

program: expr ;

path: IDENT { $$ = vector_init($1); }
    | path COLON2 IDENT { vector_push(&$1, $3); $$ = $1; }
    ;

type: path { $$ = ast_type(x, @$, $1); }
    | MUL type { $$ = ast_pointer(x, @$, $2); }
    | BITAND type { $$ = ast_reference(x, @$, $2); }
    ;

primary: path { $$ = ast_name(x, @$, $1); }
    | INTEGER { $$ = ast_digit(x, @$, $1); }
    | BOOLEAN { $$ = ast_boolean(x, @$, $1); }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

postfix: primary { $$ = $1; }
    | postfix AS type { $$ = ast_cast(x, @$, $1, $3); }
    ;

unary: postfix { $$ = $1; }
    | ADD unary { $$ = ast_unary(x, @$, $2, UNARY_ABS); }
    | SUB unary { $$ = ast_unary(x, @$, $2, UNARY_NEG); }
    ;

mul: unary { $$ = $1; }
    | mul MUL unary { $$ = ast_binary(x, @$, $1, $3, BINARY_MUL); }
    | mul DIV unary { $$ = ast_binary(x, @$, $1, $3, BINARY_DIV); }
    | mul MOD unary { $$ = ast_binary(x, @$, $1, $3, BINARY_REM); }
    ;

add: mul { $$ = $1; }
    | add ADD mul { $$ = ast_binary(x, @$, $1, $3, BINARY_ADD); }
    | add SUB mul { $$ = ast_binary(x, @$, $1, $3, BINARY_SUB); }
    ;

compare: add { $$ = $1; }
    ;

equality: compare { $$ = $1; }
    ;

shift: equality { $$ = $1; }
    | shift LSHIFT equality { $$ = ast_binary(x, @$, $1, $3, BINARY_SHL); }
    | shift RSHIFT equality { $$ = ast_binary(x, @$, $1, $3, BINARY_SHR); }
    ;

bits: shift { $$ = $1; }
    | bits BITAND shift { $$ = ast_binary(x, @$, $1, $3, BINARY_AND); }
    | bits BITOR shift { $$ = ast_binary(x, @$, $1, $3, BINARY_OR); }
    ;

xor: bits { $$ = $1; }
    | xor BITXOR bits { $$ = ast_binary(x, @$, $1, $3, BINARY_XOR); }
    ;

and: xor { $$ = $1; }
    ;

or: and { $$ = $1; }
    ;

expr: or { $$ = $1; }
    ;

%%
