grammar cthulhu;

unit
    : importDecl* bodyDecl* EOF
    ;

name
    : Ident (Colon2 Ident)*
    ;

list
    : Ident (Comma Ident)*
    ;

importSpec
    : LParen (Mul | list) RParen
    ;

importDecl
    : Import name importSpec SColon
    ;

bodyDecl
    :
    ;


// declarations
Import : 'import' ;
Type : 'type' ;
Struct : 'struct' ;
Union : 'union' ;
Enum : 'enum' ;
Def : 'def' ;
Let : 'let' ;
Var : 'var' ;

// control flow
If : 'if' ;
Else : 'else' ;
While : 'while' ;
Do : 'do' ;
For : 'for' ;
Switch : 'switch' ;
Case : 'case' ;
Continue : 'continue' ;
Break : 'break' ;
Match : 'match' ;
Default : 'default' ;
Return : 'return' ;
Cast : 'cast' ;

// types
Mut : 'mut' ;
Char : 'char' ;
UChar : 'uchar' ;
Short : 'short' ;
UShort : 'ushort' ;
Int : 'int' ;
UInt : 'uint' ;
Long : 'long' ;
ULong : 'ulong' ;
Float : 'float' ;
Double : 'double' ;
LDouble : 'ldouble' ;
Bool : 'bool' ;
Void : 'void' ;

LParen : '(' ;
RParen : ')' ;

Comma : ',' ;
Dot : '.' ;
At : '@' ;
Assign : '=' ;

Colon : ':' ;
Colon2 : '::' ;
SColon : ';' ;

Mul : '*' ;
MulEq : '*=' ;








String
    : '"' '"'
    ;

Char
    : '\'' '\''
    ;

Int10
    : NUMBER+
    ;

Int2
    : ('0b' | '0B') BIN+
    ;

Int8
    : ('0o' | '0O') OCT+
    ;

Int16
    : ('0x' | '0X') HEX+
    ;

Ident
    : LETTER (LETTER | DIGIT)*
    ;

fragment LETTER
    : [a-zA-Z_]
    ;

fragment BIN
    : [01_]
    ;

fragment OCT
    : [0-7_]
    ;

fragment HEX
    : [0-9a-fA-F_]
    ;

fragment NUMBER
    : [0-9_]
    ;

fragment DIGIT
    : [0-9]
    ;