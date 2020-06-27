grammar cthulhu;



DEF : 'def' ;
ALIAS : 'alias' ;
STRUCT : 'struct' ;
UNION : 'union' ;
OBJECT : 'object' ;
VAR : 'var' ;
GENERIC : 'generic' ;
TYPE : 'type' ;

Ident : (Alpha | '_') (Alpha | Digit | '_')* ;

Literal
    : CharLiteral
    | StringLiteral
    | IntLiteral
    ;

fragment CharLiteral : '\'' Char '\'' ;
fragment Char : (StringEscape | .) ;

fragment StringLiteral : ShortString | LongString ;
fragment ShortString : '"' (StringEscape | ~[\\\r\n\f"] )* '"';
fragment StringEscape : '\\' . ;
fragment LongString : '"""' (StringEscape | .)*? '"""';

fragment IntLiteral : (IntConst10 | IntConst2 | IntConst8 | IntConst16) NumSuffix? ;
fragment NumSuffix : ([uU] | [iI]) ('8' | '16' | '32' | '64') ;

fragment IntConst10 : Digit+ ;
fragment IntConst2 : '0b' BinDigit+ ;
fragment IntConst8 : '0o' OctDigit+ ;
fragment IntConst16 : '0x' HexDigit+ ;

fragment Digit : [0-9] ;
fragment BinDigit : [01_] ;
fragment OctDigit : [0-7_] ;
fragment HexDigit : [0-9a-fA-F_] ;
fragment Alpha : [a-zA-Z] ;

Whitespace : [ \t\r\n]+ -> skip ;

BlockComment : '/*' (BlockComment |. )*? '*/' -> skip ;

LineComment : '//' ~[\r\n]* -> skip ;