%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {json}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"

    #include "std/typed/vector.h"
    #include "std/vector.h"
}

%code requires {
    #include "json_ast.h"
    #include "json_scan.h"
    #define YYSTYPE JSONSTYPE
    #define YYLTYPE JSONLTYPE
}

%{
int jsonlex(void *lval, void *loc, scan_t *scan);
void jsonerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    mpz_t integer;
    float real;
    bool boolean;
    text_t string;

    json_t *json;
    vector_t *array;
    typevec_t *members;
    json_member_t member;
}

%start json

%token
    LBRACE "{"
    RBRACE "}"
    LBRACKET "["
    RBRACKET "]"
    COMMA ","
    COLON ":"
    NULLVAL "null"

%token<string>
    STRING

%token<integer>
    INTEGER

%token<boolean>
    BOOLEAN

%token<real>
    REAL

%type<array>
    array elements

%type<members>
    members

%type<json>
    value
    element
    object

%type<member>
    member

%%

json: element { scan_set(x, $1); }
    ;

element: value { $$ = $1; }
    ;

value: object { $$ = $1; }
    | array { $$ = json_ast_array(x, @$, $1); }
    | STRING { $$ = json_ast_string(x, @$, $1); }
    | INTEGER { $$ = json_ast_integer(x, @$, $1); }
    | REAL { $$ = json_ast_float(x, @$, $1); }
    | BOOLEAN { $$ = json_ast_boolean(x, @$, $1); }
    | NULLVAL { $$ = json_ast_null(x, @$); }
    ;

object: LBRACE RBRACE { $$ = json_ast_empty_object(x, @$); }
    | LBRACE members RBRACE { $$ = json_ast_object(x, @$, $2); }
    ;

array: LBRACKET RBRACKET { $$ = &kEmptyVector; }
    | LBRACKET elements RBRACKET { $$ = $2; }
    ;

elements: element { $$ = vector_init($1, BISON_ARENA(x)); }
    | elements COMMA element { vector_push(&$1, $3); $$ = $1; }
    ;

members: member { $$ = typevec_new(sizeof(json_member_t), 8, BISON_ARENA(x)); typevec_push($$, &$1); }
    | members COMMA member { typevec_push($1, &$3); $$ = $1; }
    ;

member: STRING COLON element { $$ = json_member($1, $3); }
    ;

%%
