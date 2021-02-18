parser grammar ZagParser;

options { tokenVocab=ZagLexer; }

root: include* (alias | struct | union | enumerate | function)* EOF ;

/* decorators */
decorate : AT expr | AT LSQUARE expr (COMMA expr)* RSQUARE ;

/* imports */
include: USING path modules? SEMI ;
path : ID (COLON2 ID)* ;
modules : LPAREN DOT3 RPAREN | LPAREN ID (COMMA ID)* RPAREN ;

/* types */
type : mutable | immutable ;
immutable : (pointer | array | qualified) closure? ;
mutable : VAR LPAREN immutable RPAREN ;
closure : LPAREN types? RPAREN ;
types : type (COMMA type)* ;
pointer : MUL type ;
array : LSQUARE type (COLON expr)? RSQUARE ;
qualified : name (COLON2 name)* ;
name : ID (BEGIN types END)? ;

/* aliasing */
alias : decorate* USING ID ASSIGN type SEMI ;

struct : decorate* STRUCT ID inherits? LBRACE fields RBRACE ;
union : decorate* UNION ID inherits? LBRACE fields RBRACE ;
enumerate : decorate* ENUM ID LBRACE items? RBRACE ;

inherits : COLON (type | LPAREN inherit (COMMA inherit)* RPAREN)  ;
inherit : ID ASSIGN type ;

items : item (COMMA item)* ;
item : ID (ASSIGN expr)? ;

fields : (field SEMI)* ;
field : (function | var | let | alias) ;

/* variables */
var : VAR names ASSIGN expr SEMI ;
let : LET names ASSIGN expr SEMI ;

names : ID | LSQUARE ID (COMMA ID)* RSQUARE ;

/* functions */
function : decorate* DEF ID params? result? body ;
params : LPAREN param (COMMA param)* RPAREN ;
param : ID COLON type ;
result : COLON type ;
body : ASSIGN expr SEMI | LBRACE stmt* RBRACE | SEMI ;


/* expressions */
expr : ternaryExpr ;

assignExpr : ternaryExpr | unaryExpr assignOp assignExpr ;
assignOp : ASSIGN | MULEQ | DIVEQ | MODEQ | ADDEQ | SUBEQ | SHLEQ | SHREQ | BITANDEQ | BITOREQ | XOREQ ;

ternaryExpr : condExpr | condExpr QUESTION ternaryExpr COLON ternaryExpr | condExpr COALESCE ternaryExpr ;

condExpr : bitExpr (condOp condExpr)* ;
condOp : AND | OR ;

bitExpr : equalExpr (bitOp bitExpr)* ;
bitOp : XOR | BITAND | BITOR ;

equalExpr : compExpr (equalOp equalExpr)* ;
equalOp : EQ | NEQ ;

compExpr : shiftExpr (compOp compExpr)* ;
compOp : GT | GTE | LT | LTE ;

shiftExpr : addExpr (shiftOp shiftExpr)* ;
shiftOp : SHL | SHR ;

addExpr : mulExpr (addOp addExpr)* ;
addOp : ADD | SUB ;

mulExpr : unaryExpr (mulOp mulExpr)* ;
mulOp : MUL | DIV | MOD ;

unaryExpr : unaryOp? postfix ;
unaryOp : MUL | BITAND | ADD | SUB | NOT | FLIP ;

postfix
    : primary
    | postfix LSQUARE expr RSQUARE
    | postfix LPAREN args? RPAREN
    | postfix DOT ID
    | postfix ARROW ID
    | primary NOT expr
    ;

primary : qualified | INT | STRING | CHAR | LPAREN expr RPAREN | cast ;

cast : CAST BEGIN type END LPAREN expr RPAREN ;

args : expr (COMMA expr)* ;

/* statements */
stmt : expr | braceStmt | withStmt | whileStmt | returnStmt | branchStmt | let | var ;
braceStmt : LBRACE stmt* RBRACE ;
withStmt : WITH LPAREN expr RPAREN stmt elseStmt? ;
whileStmt : WHILE LPAREN expr RPAREN stmt ;
forStmt : FOR LPAREN (forLoop | forRange) RPAREN stmt ;
forLoop : VAR ;
forRange : (var | let) DOT2 expr ;
returnStmt : RETURN expr? SEMI ;
branchStmt : ifStmt elifStmt* elseStmt* ;
ifStmt : IF LPAREN expr RPAREN stmt ;
elifStmt : ELSE IF LPAREN expr RPAREN stmt ;
elseStmt : ELSE stmt ;
switchStmt : SWITCH LPAREN expr RPAREN LBRACE caseStmt* defaultStmt? RBRACE ;
caseStmt : CASE expr COLON stmt ;
defaultStmt : ELSE COLON stmt ;
