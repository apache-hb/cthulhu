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
    bool boolean;
    text_t string;
}

%type<vector>
    importList importBodyList moduleList
    declSeq decl
    identList
    valueSeq valueDecl valueDeclSeq
    constSeq constDeclSeq
    typeSeq typeDeclSeq
    fieldList fieldDecl

    optParams params
    paramList paramDecl
    stmtSeq optInit
    exprList optExprList

%type<ast>
    importBody module
    type forward

    stmt

    constDecl typeDecl procDecl
    constExpr expr optExpr
    designator factor term simpleExpr simpleExprInner qualified

    optReceiver receiver returnType

%type<symbol> identDef
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

program: moduleList { scan_set(x, $1); }
    ;

moduleList: module { $$ = vector_init($1); }
    | moduleList module { vector_push(&$1, $2); $$ = $1; }
    ;

module: MODULE IDENT SEMI importList declSeq optInit end DOT { $$ = obr_module(x, @$, $2, $7, $4, $5, $6); }
    ;

optInit: %empty { $$ = NULL; }
    | START stmtSeq { $$ = $2; }
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
    | typeDeclSeq { $$ = $1; }
    | forward SEMI { $$ = vector_init($1); }
    | procDecl SEMI { $$ = vector_init($1); }
    ;

/* procedures */

procDecl: PROCEDURE optReceiver identDef optParams returnType SEMI declSeq START stmtSeq end
    {
        $$ = obr_decl_procedure(x, @$, $3, $2, $4, $5, $7, $9, $10);
    }
    ;

forward: PROCEDURE CARET optReceiver identDef optParams returnType
    {
        $$ = obr_decl_procedure(x, @$, $4, $3, $5, $6, NULL, NULL, NULL);
    }
    ;

returnType: %empty { $$ = NULL; }
    | COLON type { $$ = $2; }
    ;

optReceiver: %empty { $$ = NULL; }
    | receiver { $$ = $1; }
    ;

receiver: LPAREN mut IDENT COLON IDENT RPAREN { $$ = obr_receiver(x, @$, $2, $3, $5); }
    ;

optParams: %empty { $$ = vector_of(0); }
    | params { $$ = $1; }
    ;

params: LPAREN paramList RPAREN { $$ = $2; }
    ;

paramList: paramDecl { $$ = $1; }
    | paramList SEMI paramDecl { vector_append(&$1, $3); $$ = $1; }
    ;

paramDecl: mut identList COLON type { $$ = obr_expand_params($2, $4, $1); }
    ;

mut: %empty { $$ = false; }
    | VAR { $$ = true; }
    ;

/* consts */

constDeclSeq: CONST constSeq { $$ = $2; }
    ;

constSeq: constDecl { $$ = vector_init($1); }
    | constSeq constDecl { vector_push(&$1, $2); $$ = $1; }
    ;

constDecl: identDef EQUAL constExpr SEMI { $$ = obr_decl_const(x, @$, $1, $3); }
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

typeDeclSeq: TYPE typeSeq { $$ = $2; }
    ;

typeSeq: typeDecl { $$ = vector_init($1); }
    | typeSeq typeDecl { vector_push(&$1, $2); $$ = $1; }
    ;

typeDecl: identDef EQUAL type SEMI { $$ = obr_decl_type(x, @$, $1, $3); }
    ;

type: qualified { $$ = $1; }
    | POINTER TO type { $$ = obr_type_pointer(x, @$, $3); }
    | ARRAY optExprList OF type { $$ = obr_type_array(x, @$, $2, $4); }
    | RECORD fieldList END { $$ = obr_type_record(x, @$, $2); }
    ;

qualified: IDENT { $$ = obr_type_name(x, @$, $1); }
    | IDENT DOT IDENT { $$ = obr_type_qual(x, @$, $1, $3); }
    ;

/* record fields */

fieldList: fieldDecl { $$ = $1; }
    | fieldList SEMI fieldDecl { vector_append(&$1, $3); $$ = $1; }
    ;

fieldDecl: identList COLON type { $$ = obr_expand_fields($1, $3); }
    ;

/* statements */

stmtSeq: stmt { $$ = vector_init($1); }
    | stmtSeq SEMI stmt { vector_push(&$1, $3); $$ = $1; }
    ;

stmt: RETURN optExpr { $$ = obr_stmt_return(x, @$, $2); }
    | WHILE expr DO stmtSeq END { $$ = obr_stmt_while(x, @$, $2, $4); }
    | designator ASSIGN expr { $$ = obr_stmt_assign(x, @$, $1, $3); }
    | designator LPAREN optExprList RPAREN { $$ = obr_expr_call(x, @$, $1, $3); }
    ;

/* exprs */

designator: IDENT { $$ = obr_expr_name(x, @$, $1); } /* this deviates from the original grammar to prevent ambiguity, this isnt a breaking change */
    | designator DOT IDENT { $$ = obr_expr_field(x, @$, $1, $3); }
    //| designator LPAREN qualified RPAREN { $$ = obr_expr_cast(x, @$, $1, $3); } /* TODO: oberons cast syntax is great i just love it */
    ;

factor: designator { $$ = $1; }
    | designator LPAREN optExprList RPAREN { $$ = obr_expr_call(x, @$, $1, $3); }
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

simpleExpr: simpleExprInner { $$ = $1; }
    | PLUS simpleExprInner { $$ = obr_expr_unary(x, @$, eUnaryAbs, $2); }
    | MINUS simpleExprInner { $$ = obr_expr_unary(x, @$, eUnaryNeg, $2); }
    ;

/* extract the inner simple expr to simplify the grammar */
simpleExprInner: term { $$ = $1; }
    | simpleExprInner PLUS term { $$ = obr_expr_binary(x, @$, eBinaryAdd, $1, $3); }
    | simpleExprInner MINUS term { $$ = obr_expr_binary(x, @$, eBinarySub, $1, $3); }
    | simpleExprInner OR term { $$ = obr_expr_binary(x, @$, eBinaryBitOr, $1, $3); }
    ;

expr: simpleExpr { $$ = $1; }
    | simpleExpr EQUAL simpleExpr { $$ = obr_expr_compare(x, @$, eCompareEq, $1, $3); }
    | simpleExpr NEQUAL simpleExpr { $$ = obr_expr_compare(x, @$, eCompareNeq, $1, $3); }
    | simpleExpr LT simpleExpr { $$ = obr_expr_compare(x, @$, eCompareLt, $1, $3); }
    | simpleExpr GT simpleExpr { $$ = obr_expr_compare(x, @$, eCompareGt, $1, $3); }
    | simpleExpr LTE simpleExpr { $$ = obr_expr_compare(x, @$, eCompareLte, $1, $3); }
    | simpleExpr GTE simpleExpr { $$ = obr_expr_compare(x, @$, eCompareGte, $1, $3); }
    | simpleExpr IN simpleExpr { $$ = obr_expr_in(x, @$, $1, $3); }
    | simpleExpr IS simpleExpr { $$ = obr_expr_is(x, @$, $1, $3); }
    ;

constExpr: expr { $$ = $1; }
    ;

optExpr: %empty { $$ = NULL; }
    | expr { $$ = $1; }
    ;

/* extra */

optExprList: %empty { $$ = vector_of(0); }
    | exprList { $$ = $1; }
    ;

exprList: expr { $$ = vector_init($1); }
    | exprList COMMA expr { vector_push(&$1, $3); $$ = $1; }
    ;

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