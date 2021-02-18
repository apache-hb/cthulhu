lexer grammar ZagLexer;

STRUCT : 'struct' ;
UNION : 'union' ;
ENUM : 'enum' ;
DEF : 'def' ;
CAST : 'cast' ;
USING : 'using' ;
VAR : 'var' ;
LET : 'let' ;
EXTERN : 'extern' ;
STATIC : 'static' ;
VIRTUAL : 'virtual' ;

WITH : 'with' ;
RETURN : 'return' ;
WHILE : 'while' ;
FOR : 'for' ;
CONTINUE : 'continue' ;
BREAK : 'break' ;
SWITCH : 'switch' ;
CASE : 'case' ;
IF : 'if' ;
ELSE : 'else' ;
ASM : 'asm' ;


AT : '@' ;
SEMI : ';' ;
COLON : ':' ;
COLON2 : '::' ;
ASSIGN : '=' ;
ARROW : '->' ;
COALESCE : '?:' ;
QUESTION : '?' ;

LPAREN : '(' ;
RPAREN : ')' ;
LSQUARE : '[' ;
RSQUARE : ']' ;
LBRACE : '{' ;
RBRACE : '}' ;

COMMA : ',' ;
DOT : '.' ;
DOT2 : '..' ;
DOT3 : '...' ;

BEGIN : '!<' ;
END : '>' ;

ADD : '+' ;
ADDEQ : '+=' ;
SUB : '-' ;
SUBEQ : '-=' ;
MUL : '*' ;
MULEQ : '*=' ;
DIV : '/' ;
DIVEQ : '/=' ;
MOD : '%' ;
MODEQ : '%=' ;
XOR : '^' ;
XOREQ : '^=' ;

SHL : '<<' ;
SHLEQ : '<<=' ;
SHR : '>>' ;
SHREQ : '>>=' ;

AND : '&&' ;
OR : '||' ;

BITAND : '&' ;
BITANDEQ : '&=' ;
BITOR : '|' ;
BITOREQ : '|=' ;
FLIP : '~' ;

NOT : '!' ;
EQ : '==' ;
NEQ : '!=' ;

LT : '<' ;
LTE : '<='; 
GT : END ;
GTE : '>=' ;

INT : [0-9]+ ;
STRING : '"' LETTER* '"' ;
CHAR : '\'' ~['\\\r\n] '\'';

fragment LETTER : '\\' ['"ntv0\\] | ~[\\\r\n] ;

ID : [a-zA-Z_][a-zA-Z0-9_]* ;
WS : [ \t\r\n]+ -> channel(HIDDEN) ;