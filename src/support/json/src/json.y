%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {json}

%code top {
    #include "json_actions.h"
    #include "interop/bison.h"

    #include "arena/arena.h"
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
void jsonerror(json_where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    mpz_t integer;
    float real;
    bool boolean;
    text_view_t string;

    json_t json;
    typevec_t array;
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
    elements array

%type<members>
    members

%type<json>
    value
    element
    object

%type<member>
    member

%%

json: element { scan_set(x, arena_memdup(&$1, sizeof(json_t), BISON_ARENA(x))); }
    ;

element: value { $$ = $1; }
    ;

value: object { $$ = $1; }
    | array { $$ = json_ast_array(@$.where, $1); }
    | STRING { $$ = json_ast_string(@$.where, $1); }
    | INTEGER { $$ = json_ast_integer(@$.where, $1); }
    | REAL { $$ = json_ast_float(@$.where, $1); }
    | BOOLEAN { $$ = json_ast_boolean(@$.where, $1); }
    | NULLVAL { $$ = json_ast_null(@$.where); }
    ;

object: LBRACE RBRACE { $$ = json_ast_empty_object(@$.where); }
    | LBRACE members RBRACE { $$ = json_ast_object(x, @$.where, $2); }
    ;

array: LBRACKET RBRACKET { $$ = kEmptyTypevec; }
    | LBRACKET elements RBRACKET { $$ = $2; }
    ;

elements: element {
        typevec_t vec = typevec_make(sizeof(json_t), 4, BISON_ARENA(x));
        typevec_push(&vec, &$1);
        $$ = vec;
    }
    | elements COMMA element { typevec_push(&$1, &$3); $$ = $1; }
    ;

members: member { $$ = typevec_new(sizeof(json_member_t), 8, BISON_ARENA(x)); typevec_push($$, &$1); }
    | members COMMA member { typevec_push($1, &$3); $$ = $1; }
    ;

member: STRING COLON element { $$ = json_member($1, $3); }
    ;

%%
