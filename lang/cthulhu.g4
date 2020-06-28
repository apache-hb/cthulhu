grammar cthulhu;

unit : importDecl* bodyDecl* ;

importDecl : 'import' path importSpec? NL ;

importSpec : '(' ID (',' ID)* ')' ;

bodyDecl : aliasDecl | structDecl | varDecl ;

varDecl : decorator* 'var' ID (':' type | ':' type '=' expr | '=' expr) NL ;

structField : decorator* ID ':' type NL ;

structDecl : decorator* 'struct' ID '{' structField* '}' ;

aliasDecl : 'alias' ID '=' type NL ;

type : '&'? typebody | '*' type | decorator type | '[' (expr | 'var') ']' type | 'def' '(' types? ')' '->' type;

types : type (',' type)* ;

typebody : path ;

path : ID ('::' ID)* ;

decorator : '@' decoratorItem | '@' '[' decoratorItem (',' decoratorItem)* ']' ;

decoratorItem : path ('(' args? ')')? ;

args : expr (',' expr)* ;

unary : ('+' | '-' | '*' | '&' | '!' | '~') expr ;

binop : '+' | '-' | '*' | '&' | '/' | '|' | '%' | '^' | '.' | '->' ;

initBody : initItem (',' initItem)* ;

initItem : '[' expr ']' '=' expr ;

expr : (path | NUMBER | STR | CHAR | unary | '(' expr ')') ((binop expr) | '?' expr ':' expr | '(' args ')')? | path? '{' initBody '}' ;

NL : ';' ;


NUMBER : HEX | BIN | OCT | NUM ;
HEX : '0x' [0-9a-fA-F_]+ ;
BIN : '0b' [01_]+ ;
OCT : '0o' [0-7_]+ ;

STR : '"' .*? '"' ;
CHAR : '\'' . '\'' ;

// no leading zeros aside from the number 0
NUM : '0' | [1-9][0-9_]* ;

ID : [a-zA-Z_][a-zA-Z0-9_]* ;
WS : [ \t\r\n]+ -> skip;