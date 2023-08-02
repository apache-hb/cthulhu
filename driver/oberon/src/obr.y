%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {obr}

%code top {
    #include "interop/flex.h"
}

%code requires {
    #include "oberon/scan.h"
    #include "oberon/ast.h"

    #define YYSTYPE OBRSTYPE
    #define YYLTYPE OBRLTYPE
}

%{
int obrlex(void *lval, void *loc, scan_t *scan);
void obrerror(where_t *where, void *state, scan_t *scan, const char *msg);
%}

%union {
    obr_t *ast;
    obr_partial_value_t *partialValue;

    vector_t *vector;

    char *ident;
    mpz_t number;
}

%type<vector>
    importList importBodyList moduleList
    declSeq decl
    nameList valueSeq valueDecl valueDeclSeq

%type<ast>
    importBody module
    type

%type<partialValue>
    valueName

%type<ident>
    end

%token<ident>
    IDENT "identifier"

%token<number>
    NUMBER "number"

%token
    PLUS "+"
    MINUS "-"
    DIVIDE "/"
    STAR "*"

    TILDE "~"
    CARET "^"
    AND "&"
    BAR "|"
    COLON ":"

    EQUAL "="
    NEQUAL "#"

    DOT "."
    DOT2 ".."

    COMMA ","
    SEMI ";"

    LT "<"
    GT ">"
    LTE "<="
    GTE ">="

    LPAREN "("
    RPAREN ")"

    LBRACKET "["
    RBRACKET "]"

    LBRACE "{"
    RBRACE "}"

    ASSIGN ":="

%token
    ARRAY "ARRAY"
    START "BEGIN"
    BY "BY"
    CASE "CASE"
    CONST "CONST"
    DIV "DIV"
    DO "DO"
    ELSE "ELSE"
    ELSIF "ELSIF"
    END "END"
    EXIT "EXIT"
    FOR "FOR"
    IF "IF"
    IMPORT "IMPORT"
    IN "IN"
    IS "IS"
    LOOP "LOOP"
    MOD "MOD"
    MODULE "MODULE"
    NIL "NIL"
    OF "OF"
    OR "OR"
    POINTER "POINTER"
    PROCEDURE "PROCEDURE"
    RECORD "RECORD"
    REPEAT "REPEAT"
    RETURN "RETURN"
    THEN "THEN"
    TO "TO"
    TYPE "TYPE"
    UNTIL "UNTIL"
    VAR "VAR"
    WHILE "WHILE"
    WITH "WITH"

%start program

%%

program: moduleList { scan_set(x, $1); }
    ;

moduleList: module { $$ = vector_init($1); }
    | moduleList module { vector_push(&$1, $2); $$ = $1; }
    ;

module: MODULE IDENT SEMI importList declSeq end DOT { $$ = obr_module(x, @$, $2, $6, $4, $5); }
    ;

/* imports */

importList: %empty { $$ = vector_of(0); }
    | IMPORT importBodyList SEMI { $$ = $2; }
    ;

importBodyList: importBody { $$ = vector_init($1); }
    | importBodyList COMMA importBody { vector_push(&$1, $3); $$ = $1; }
    ;

importBody: IDENT { $$ = obr_import(x, @$, $1, $1); }
    | IDENT ASSIGN IDENT { $$ = obr_import(x, @$, $1, $3); }
    ;

/* values */

declSeq: decl { $$ = $1; }
    | declSeq decl { vector_append(&$1, $2); $$ = $1; }
    ;

decl: valueDeclSeq { $$ = $1; }
    ;

valueDeclSeq: VAR valueSeq { $$ = $2; }
    ;

valueSeq: valueDecl { $$ = $1; }
    | valueSeq valueDecl { vector_append(&$1, $2); $$ = $1; }
    ;

valueDecl: nameList COLON type SEMI { $$ = obr_expand_values(false, $1, $3); }
    ;

nameList: valueName { $$ = vector_init($1); }
    | nameList COMMA valueName { vector_push(&$1, $3); $$ = $1; }
    ;

valueName: IDENT { $$ = obr_partial_value(x, @$, $1); }
    ;

/* types */

type: IDENT { $$ = obr_type_name(x, @$, $1); }
    | IDENT DOT IDENT { $$ = obr_type_qual(x, @$, $1, $3); }
    ;

/* exprs */

/* extra */

end: END { $$ = NULL; }
    | END IDENT { $$ = $2; }
    ;

%%
