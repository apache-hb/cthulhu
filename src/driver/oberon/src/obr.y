%define parse.error verbose
%define api.pure full
%lex-param { void *scan }
%parse-param { void *scan } { scan_t *x }
%locations
%expect 0
%define api.prefix {obr}

%code top {
    #include "interop/flex.h"
    #include "interop/bison.h"
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
    bool boolean;
    text_t string;
}

%type<vector>
    importList import_body_list module_seq
    decl_seq decl
    ident_list
    value_seq value_decl value_decl_seq
    const_seq const_decl_seq
    type_seq type_decl_seq
    field_list field_decl

    opt_params params
    param_list param_decl
    stmt_seq opt_init
    expr_list opt_expr_list

%type<ast>
    import_body module
    type forward

    stmt
    branch_stmt branch_tail

    const_decl type_decl proc_decl
    const_expr expr opt_expr
    designator factor term simple_expr simple_expr_inner qualified

    optReceiver receiver return_type

%type<symbol> ident_def
%type<ident> end
%type<boolean> mut

%token<ident> IDENT "identifier"
%token<number> NUMBER "numeric literal"
%token<string> STRING "string literal"

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

program: module_seq { scan_set(x, $1); }
    ;

module_seq: module { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    | module_seq module { vector_push(&$1, $2); $$ = $1; }
    ;

module: MODULE IDENT SEMI importList decl_seq opt_init end DOT { $$ = obr_module(x, @$, $2, $7, $4, $5, $6); }
    ;

opt_init: %empty { $$ = NULL; }
    | START stmt_seq { $$ = $2; }
    ;

/* imports */

importList: %empty { $$ = &kEmptyVector; }
    | IMPORT import_body_list SEMI { $$ = $2; }
    ;

import_body_list: import_body { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    | import_body_list COMMA import_body { vector_push(&$1, $3); $$ = $1; }
    ;

import_body: IDENT { $$ = obr_import(x, @$, $1, $1); }
    | IDENT ASSIGN IDENT { $$ = obr_import(x, @$, $1, $3); }
    ;

/* values */

decl_seq: decl { $$ = $1; }
    | decl_seq decl { vector_append(&$1, $2); $$ = $1; }
    ;

decl: value_decl_seq { $$ = $1; }
    | const_decl_seq { $$ = $1; }
    | type_decl_seq { $$ = $1; }
    | forward SEMI { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    | proc_decl SEMI { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    ;

/* procedures */

proc_decl: PROCEDURE optReceiver ident_def opt_params return_type SEMI decl_seq START stmt_seq end
    {
        $$ = obr_decl_procedure(x, @$, $3, $2, $4, $5, $7, $9, $10);
    }
    ;

forward: PROCEDURE CARET optReceiver ident_def opt_params return_type
    {
        $$ = obr_decl_procedure(x, @$, $4, $3, $5, $6, NULL, NULL, NULL);
    }
    ;

return_type: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

optReceiver: %empty { $$ = NULL; }
    | receiver { $$ = $1; }
    ;

receiver: LPAREN mut IDENT COLON IDENT RPAREN { $$ = obr_receiver(x, @$, $2, $3, $5); }
    ;

opt_params: %empty { $$ = &kEmptyVector; }
    | params { $$ = $1; }
    ;

params: LPAREN param_list RPAREN { $$ = $2; }
    ;

param_list: param_decl { $$ = $1; }
    | param_list SEMI param_decl { vector_append(&$1, $3); $$ = $1; }
    ;

param_decl: mut ident_list COLON type { $$ = obr_expand_params($2, $4, $1); }
    ;

mut: %empty { $$ = false; }
    | VAR { $$ = true; }
    ;

/* consts */

const_decl_seq: CONST const_seq { $$ = $2; }
    ;

const_seq: const_decl { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    | const_seq const_decl { vector_push(&$1, $2); $$ = $1; }
    ;

const_decl: ident_def EQUAL const_expr SEMI { $$ = obr_decl_const(x, @$, $1, $3); }
    ;

/* values */

value_decl_seq: VAR value_seq { $$ = $2; }
    ;

value_seq: value_decl { $$ = $1; }
    | value_seq value_decl { vector_append(&$1, $2); $$ = $1; }
    ;

value_decl: ident_list COLON type SEMI { $$ = obr_expand_vars($1, $3); }
    ;

/* types */

type_decl_seq: TYPE type_seq { $$ = $2; }
    ;

type_seq: type_decl { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    | type_seq type_decl { vector_push(&$1, $2); $$ = $1; }
    ;

type_decl: ident_def EQUAL type SEMI { $$ = obr_decl_type(x, @$, $1, $3); }
    ;

type: qualified { $$ = $1; }
    | POINTER TO type { $$ = obr_type_pointer(x, @$, $3); }
    | ARRAY opt_expr_list OF type { $$ = obr_type_array(x, @$, $2, $4); }
    | RECORD field_list END { $$ = obr_type_record(x, @$, $2); }
    ;

qualified: IDENT { $$ = obr_type_name(x, @$, $1); }
    | IDENT DOT IDENT { $$ = obr_type_qual(x, @$, $1, $3); }
    ;

/* record fields */

field_list: field_decl { $$ = $1; }
    | field_list SEMI field_decl { vector_append(&$1, $3); $$ = $1; }
    ;

field_decl: ident_list COLON type { $$ = obr_expand_fields($1, $3); }
    ;

/* statements */

stmt_seq: stmt { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    | stmt_seq SEMI stmt { vector_push(&$1, $3); $$ = $1; }
    ;

stmt: RETURN opt_expr { $$ = obr_stmt_return(x, @$, $2); }
    | WHILE expr DO stmt_seq END { $$ = obr_stmt_while(x, @$, $2, $4); }
    | LOOP stmt_seq END { $$ = obr_stmt_loop(x, @$, $2); }
    | designator ASSIGN expr { $$ = obr_stmt_assign(x, @$, $1, $3); }
    | designator LPAREN opt_expr_list RPAREN { $$ = obr_expr_call(x, @$, $1, $3); }
    | REPEAT stmt_seq UNTIL expr { $$ = obr_stmt_repeat(x, @$, $2, $4); }
    | EXIT { $$ = obr_stmt_break(x, @$); }
    | branch_stmt { $$ = $1; }
    ;

branch_stmt: IF expr THEN stmt_seq branch_tail END { $$ = obr_stmt_branch(x, @$, $2, $4, $5); }
    ;

branch_tail: %empty { $$ = NULL; }
    | ELSIF expr THEN stmt_seq branch_tail { $$ = obr_stmt_branch(x, @$, $2, $4, $5); }
    | ELSE stmt_seq { $$ = obr_stmt_block(x, @$, $2); }
    ;

/* exprs */

designator: IDENT { $$ = obr_expr_name(x, @$, $1); } /* this deviates from the original grammar to prevent ambiguity, this isnt a breaking change */
    | designator DOT IDENT { $$ = obr_expr_field(x, @$, $1, $3); }
    ;

factor: designator { $$ = $1; }
    | designator LPAREN opt_expr_list RPAREN { $$ = obr_expr_call(x, @$, $1, $3); }
    | NUMBER { $$ = obr_expr_digit(x, @$, $1); }
    | STRING { $$ = obr_expr_string(x, @$, $1.text, $1.size); }
    | LPAREN expr RPAREN { $$ = $2; }
    ;

term: factor { $$ = $1; }
    | term STAR factor { $$ = obr_expr_binary(x, @$, eBinaryMul, $1, $3); }
    | term DIVIDE factor { $$ = obr_expr_binary(x, @$, eBinaryDiv, $1, $3); } /* TODO: whats the difference between `/` and DIV? */
    | term DIV factor { $$ = obr_expr_binary(x, @$, eBinaryDiv, $1, $3); }
    | term MOD factor { $$ = obr_expr_binary(x, @$, eBinaryRem, $1, $3); }
    | term AND factor { $$ = obr_expr_binary(x, @$, eBinaryBitAnd, $1, $3); }
    ;

simple_expr: simple_expr_inner { $$ = $1; }
    | PLUS simple_expr_inner { $$ = obr_expr_unary(x, @$, eUnaryAbs, $2); }
    | MINUS simple_expr_inner { $$ = obr_expr_unary(x, @$, eUnaryNeg, $2); }
    ;

/* extract the inner simple expr to simplify the grammar */
simple_expr_inner: term { $$ = $1; }
    | simple_expr_inner PLUS term { $$ = obr_expr_binary(x, @$, eBinaryAdd, $1, $3); }
    | simple_expr_inner MINUS term { $$ = obr_expr_binary(x, @$, eBinarySub, $1, $3); }
    | simple_expr_inner OR term { $$ = obr_expr_binary(x, @$, eBinaryBitOr, $1, $3); }
    ;

expr: simple_expr { $$ = $1; }
    | simple_expr EQUAL simple_expr { $$ = obr_expr_compare(x, @$, eCompareEq, $1, $3); }
    | simple_expr NEQUAL simple_expr { $$ = obr_expr_compare(x, @$, eCompareNeq, $1, $3); }
    | simple_expr LT simple_expr { $$ = obr_expr_compare(x, @$, eCompareLt, $1, $3); }
    | simple_expr GT simple_expr { $$ = obr_expr_compare(x, @$, eCompareGt, $1, $3); }
    | simple_expr LTE simple_expr { $$ = obr_expr_compare(x, @$, eCompareLte, $1, $3); }
    | simple_expr GTE simple_expr { $$ = obr_expr_compare(x, @$, eCompareGte, $1, $3); }
    | simple_expr IN simple_expr { $$ = obr_expr_in(x, @$, $1, $3); }
    | simple_expr IS simple_expr { $$ = obr_expr_is(x, @$, $1, $3); }
    ;

const_expr: expr { $$ = $1; }
    ;

opt_expr: %empty { $$ = NULL; }
    | expr { $$ = $1; }
    ;

/* extra */

opt_expr_list: %empty { $$ = &kEmptyVector; }
    | expr_list { $$ = $1; }
    ;

expr_list: expr { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    | expr_list COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

ident_list: ident_def { $$ = vector_init_arena($1, BISON_ARENA(x)); }
    | ident_list COMMA ident_def { vector_push(&$1, $3); $$ = $1; }
    ;

ident_def: IDENT { $$ = obr_symbol(x, @$, $1, eObrVisPrivate); }
    | IDENT STAR { $$ = obr_symbol(x, @$, $1, eObrVisPublic); }
    | IDENT MINUS { $$ = obr_symbol(x, @$, $1, eObrVisPublicReadOnly); }
    ;

end: END { $$ = NULL; }
    | END IDENT { $$ = $2; }
    ;

%%

/** https://en.wikipedia.org/wiki/Oberon-2#Syntax
Module        = MODULE ident ";" [ImportList] DeclSeq [BEGIN StatementSeq] END ident ".".
ImportList    = IMPORT [ident ":="] ident {"," [ident ":="] ident} ";".
DeclSeq       = { CONST {ConstDecl ";" } | TYPE {TypeDecl ";"} | VAR {VarDecl ";"}} {ProcDecl ";" | ForwardDecl ";"}.
ConstDecl     = IdentDef "=" ConstExpr.
TypeDecl      = IdentDef "=" Type.
VarDecl       = IdentList ":" Type.
ProcDecl      = PROCEDURE [Receiver] IdentDef [FormalPars] ";" DeclSeq [BEGIN StatementSeq] END ident.
ForwardDecl   = PROCEDURE "^" [Receiver] IdentDef [FormalPars].
FormalPars    = "(" [FPSection {";" FPSection}] ")" [":" Qualident].
FPSection     = [VAR] ident {"," ident} ":" Type.
Receiver      = "(" [VAR] ident ":" ident ")".
Type          = Qualident
              | ARRAY [ConstExpr {"," ConstExpr}] OF Type
              | RECORD ["("Qualident")"] FieldList {";" FieldList} END
              | POINTER TO Type
              | PROCEDURE [FormalPars].
FieldList     = [IdentList ":" Type].
StatementSeq  = Statement {";" Statement}.
Statement     = [ Designator ":=" Expr
              | Designator ["(" [ExprList] ")"]
              | IF Expr THEN StatementSeq {ELSIF Expr THEN StatementSeq} [ELSE StatementSeq] END
              | CASE Expr OF Case {"|" Case} [ELSE StatementSeq] END
              | WHILE Expr DO StatementSeq END
              | REPEAT StatementSeq UNTIL Expr
              | FOR ident ":=" Expr TO Expr [BY ConstExpr] DO StatementSeq END
              | LOOP StatementSeq END
              | WITH Guard DO StatementSeq {"|" Guard DO StatementSeq} [ELSE StatementSeq] END
              | EXIT
              | RETURN [Expr]
      ].
Case          = [CaseLabels {"," CaseLabels} ":" StatementSeq].
CaseLabels    = ConstExpr [".." ConstExpr].
Guard         = Qualident ":" Qualident.
ConstExpr     = Expr.
Expr          = SimpleExpr [Relation SimpleExpr].
SimpleExpr    = ["+" | "-"] Term {AddOp Term}.
Term          = Factor {MulOp Factor}.
Factor        = Designator ["(" [ExprList] ")"] | number | character | string | NIL | Set | "(" Expr ")" | "~" Factor.
Set           = "{" [Element {"," Element}] "}".
Element       = Expr [".." Expr].
Relation      = "=" | "#" | "<" | "<=" | ">" | ">=" | IN | IS.
AddOp         = "+" | "-" | OR.
MulOp         = "*" | "/" | DIV | MOD | "&".
Designator    = Qualident {"." ident | "[" ExprList "]" | "^" | "(" Qualident ")"}.
ExprList      = Expr {"," Expr}.
IdentList     = IdentDef {"," IdentDef}.
Qualident     = [ident "."] ident.
IdentDef      = ident ["*" | "-"].
*/