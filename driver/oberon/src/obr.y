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
    obr_symbol_t *symbol;

    vector_t *vector;

    char *ident;
    mpz_t number;
}

%type<vector>
    importList importBodyList moduleList
    declSeq decl
    identList
    valueSeq valueDecl valueDeclSeq
    constSeq constDeclSeq

%type<ast>
    importBody module
    type

    constDecl
    constExpr expr

%type<symbol>
    identDef

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
    | constDeclSeq { $$ = $1; }
    ;

/* consts */

constDeclSeq: CONST constSeq { $$ = $2; }
    ;

constSeq: constDecl { $$ = vector_init($1); }
    | constSeq constDecl { vector_push(&$1, $2); $$ = $1; }
    ;

constDecl: identDef COLON type EQUAL constExpr SEMI { $$ = obr_decl_const(x, @$, $1, $3, $5); }
    ;

/* values */

valueDeclSeq: VAR valueSeq { $$ = $2; }
    ;

valueSeq: valueDecl { $$ = $1; }
    | valueSeq valueDecl { vector_append(&$1, $2); $$ = $1; }
    ;

valueDecl: identList COLON type SEMI { $$ = obr_expand_vars($1, $3); }
    ;

/* types */

type: IDENT { $$ = obr_type_name(x, @$, $1); }
    | IDENT DOT IDENT { $$ = obr_type_qual(x, @$, $1, $3); }
    ;

/* exprs */

constExpr: expr { $$ = $1; }
    ;

expr: IDENT { $$ = NULL; /* dont care yet */ }
    ;

/* extra */

identList: identDef { $$ = vector_init($1); }
    | identList COMMA identDef { vector_push(&$1, $3); $$ = $1; }
    ;

identDef: IDENT { $$ = obr_symbol(x, @$, $1, eObrVisPrivate); }
    | IDENT STAR { $$ = obr_symbol(x, @$, $1, eObrVisPublic); }
    | IDENT MINUS { $$ = obr_symbol(x, @$, $1, eObrVisPublicReadOnly); }
    ;

end: END { $$ = NULL; }
    | END IDENT { $$ = $2; }
    ;

%%
