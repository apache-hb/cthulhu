%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {cmd}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "protobuf.h"
    #define YYSTYPE CMDSTYPE
    #define YYLTYPE CMDLTYPE
}

%{
int pblex();
void pberror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    char *ident;
    uint_least32_t number;
    ast_t *ast;
    vector_t *vec;
    field_kind_t field;
}

%token<ident>
    IDENT "identifier"
    STRING "string"

%token<number>
    NUMBER "number"

%token
    ASSIGN "="
    SEMI ";"
    
    LBRACE "{"
    RBRACE "}"

    SYNTAX "syntax"
    MESSAGE "message"
    REPEATED "repeated"
    OPTIONAL "optional"
    ONEOF "oneof"
    ENUM "enum"

%type<ast>
    message field

%type<vec>
    messages fields

%type<field>
    modifier

%start entry

%%

entry: proto3 messages 
    ;

proto3: SYNTAX ASSIGN STRING SEMI
    ;

messages: message
    | messages message
    ;

message: MESSAGE IDENT LBRACE fields RBRACE
    ;

fields: field
    | fields field
    ;

field: modifier IDENT IDENT ASSIGN NUMBER SEMI { $$ = pb_field(x, @$, $3, $5, $2, $1); }
    ;

modifier: %empty { $$ = eFieldRequired; }
    | REPEATED { $$ = eFieldRepeated; }
    | OPTIONAL { $$ = eFieldOptional; }
    ;

%%
