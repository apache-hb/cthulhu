grammar cthulhu;

body : alias;

struct_field : IDENT ':' type ';' ;
struct : 'struct' IDENT '{' struct_field* '}' ;

alias : 'alias' IDENT '=' type ';' ;

name : IDENT ('::' IDENT)* ;
type : name | type ('*' | '[' ']');

WS : [ \t\r\n]+ -> skip;
COMMENT : '//' (~[\n])* ;

fragment DIGIT : [0-9] ;
fragment ALPHA : [a-zA-Z] ;

IDENT : (ALPHA | DIGIT | '_')+ ;