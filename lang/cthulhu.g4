grammar cthulhu;

unit : importDecl* bodyDecl* ;

bodyDecl : aliasDecl | structDecl | unionDecl | funcDecl;

importDecl : 'import' path '(' (importSpec | '*') ')' ;
importSpec : IDENT (',' IDENT)* ;

fieldDecl : IDENT ':' type ';' ;
enumBacking : ':' type ;
enumField : IDENT ('=' expr)? ;
enumFields : enumField (',' enumField)* ;

structDecl : 'struct' IDENT '{' fieldDecl* '}' ;
unionDecl : 'union' IDENT '{' fieldDecl* '}' ;
enumDecl : 'enum' IDENT enumBacking? '{' enumFields '}' ;
aliasDecl : 'alias' IDENT '=' type ';' ;

funcDecl : 'def' IDENT funcArgs? funcRet? funcBody ;

funcBody : '=' expr | stmtList | ';' ;

funcRet : '->' type ;

funcArgs : '(' funcArgsBody? ')' ;
funcArgsBody : funcArg (',' funcArg)* ;
funcArg : IDENT ':' type ;

type : IDENT | '*' type | 'def' '(' closureArgs? ')' '->' type | '[' expr ']' type;
closureArgs : type (',' type)* ;

expr : 'a' ;
stmt : expr | stmtList ;
stmtList : '{' stmt* '}' ;

path : IDENT ('::' IDENT)* ;

WS : [ \t\r\n]+ -> skip;
COMMENT : '//' (~[\n])* ;

fragment DIGIT : [0-9] ;
fragment ALPHA : [a-zA-Z] ;

IDENT : (ALPHA | DIGIT | '_')+ ;