grammar ctu;

importKw : 'import' ;
typeKw : 'type' ;
defKw : 'def' ;

enumKw : 'enum' ;
unionKw : 'union' ;
variantKw : 'variant' ;

whileKw : 'while' ;
returnKw : 'return' ;
ifKw : 'if' ;
elseKw : 'else' ;
matchKw : 'match' ;

colonKw : ':' ;
assignKw : ':=' ;
atKw : '@' ;
dotKw : '.' ;
commaKw : ',' ;

lparenKw : '(' ;
rparenKw : ')' ;

lbraceKw : '{' ;
rbraceKw : '}' ;

lsquareKw : '[' ;
rsquareKw : ']' ;

bigArrowKw : '=>' ;
arrowKw : '->' ;

addKw : '+' ;
subKw : '-' ;
notKw : '!' ;
bitnotKw : '~' ;

mulKw : '*' ;
divKw : '/' ;
modKw : '%' ;
questionKw : '?' ;

WS : [ \n\r\t]+ -> skip ;

ident
    : NONDIGIT (NONDIGIT | DIGIT)*
    ;

constant
    : string
    ;

string
    : '"' (ESCAPE | ~('\'' | '\\' | '\n' | '\r') ) '"'
    ;

integer
    : hexDigit
    | decimalDigit
    | binaryDigit
    ;

hexDigit
    : '0x' HEXDIGIT+
    ;

binaryDigit
    : '0b' BINDIGIT+
    ;

decimalDigit
    : DIGIT+
    ;

BINDIGIT : [0-1] ;
HEXDIGIT : [0-9a-fA-F] ;
DIGIT : [0-9] ;
NONDIGIT : [a-zA-Z_] ;
ESCAPE : '\\' ('\'' | '\\') ;


name 
    : ident (colonKw ident)* 
    ;

importDecl
    : importKw name (arrowKw ident)?
    ;



unaryOp
    : addKw
    | subKw
    | notKw
    | bitnotKw
    | mulKw
    ;

binaryOp
    : addKw
    | subKw
    | mulKw
    | divKw
    | modKw
    ;

scopeExpr
    : colonKw expr
    ;

derefExpr
    : arrowKw expr
    ;

accessExpr
    : dotKw expr
    ;

expr
    : (ident | constant)
    | unaryOp expr
    | expr scopeExpr
    | expr derefExpr
    | expr accessExpr
    | expr binaryOp expr
    ;



returnStmt
    : returnKw expr
    ;

whileStmt
    : whileKw expr stmt
    ;


elseStmt
    : elseKw stmt
    ;

elifStmt
    : elseKw ifKw expr stmt elifStmt?
    ;

ifStmt
    : ifKw expr stmt elifStmt* elseStmt?
    ;

matchStmtCase
    : expr bigArrowKw stmt
    ;

matchStmtElse
    : elseKw bigArrowKw stmt
    ;

matchStmtBody
    : lbraceKw matchStmtCase* matchStmtElse? rbraceKw
    ;

matchStmt
    : matchKw expr matchStmtBody
    ;

stmt
    : returnStmt
    | whileStmt
    | ifStmt
    | matchStmt
    | expr
    | asm
    | lbraceKw stmt* rbraceKw
    ;

asm
    : atKw 'asm' (lparenKw ('I8086' | 'X86' | 'X86_64') rparenKw)? asmBody
    ;

asmBody
    : lbraceKw asmExpr* rbraceKw
    ;

asmExpr
    : asmMov
    | asmXor
    | asmCmp
    | asmJmp
    | asmCall
    | asmRet
    ;

asmRegister
    : 'ah'
    | 'al'
    | 'ax'
    | 'eax'
    | 'rax'
    ;

asmPart
    : asmRegister
    | lsquareKw asmRegister rsquareKw
    | lsquareKw integer rsquareKw
    | lbraceKw expr rbraceKw
    ;

asmMov
    : 'mov' asmPart commaKw asmPart
    ;

asmXor
    : 'xor' asmPart commaKw asmPart
    ;

asmCmp
    : 'cmp' asmPart commaKw asmPart
    ;

asmJmp
    : 'jmp' asmPart
    ;

asmCall
    : 'call' asmPart
    ;

asmRet
    : 'ret'
    ;

funcdefBody
    : assignKw expr
    | lbraceKw stmt rbraceKw
    ;

funcdefReturn
    : arrowKw typeDecl
    ;

funcdefArgs
    : lparenKw namedTypeList? rparenKw
    ;

funcdefDecl
    : defKw ident funcdefArgs? funcdefReturn? funcdefBody
    ;






typeDecl
    : typeAttrib* typeDeclBody (arrayDecl | ptrDecl)*
    ;

typeAttrib
    : attrib
    | packedAttrib
    ;

ptrDecl
    : mulKw
    ;

arrayDecl
    : lsquareKw expr rsquareKw
    ;

typeDeclBody
    : structDecl
    | tupleDecl
    | enumDecl
    | unionDecl
    | variantDecl
    | nameDecl
    ;

structDecl
    : lbraceKw namedTypeList? rbraceKw
    ;

tupleDecl
    : lparenKw typeList? rparenKw
    ;

enumDecl
    : enumKw lbraceKw enumBody rbraceKw
    ;

unionDecl
    : unionKw structDecl
    ;

variantDecl
    : variantKw lbraceKw variantBody? rbraceKw
    ;

nameDecl
    : name
    ;


namedTypeList
    : ident colonKw typeDecl (commaKw namedTypeList)?
    ;

typeList
    : typeDecl (commaKw typeList)?
    ;

enumBody
    : ident (assignKw expr)? (commaKw enumBody)?
    ;

variantBody
    : ident (colonKw expr)? bigArrowKw typeDecl (commaKw variantBody)?
    ;


typedefDecl
    : typeKw ident assignKw typeDecl
    ;

bodyDecl
    : typedefDecl | funcdefDecl
    ;

attrib
    : atKw deprecatedAttrib
    ;


deprecatedAttrib
    : 'deprecated' lparenKw string rparenKw
    ;

packedAttrib
    : 'packed' lparenKw expr rparenKw
    ;


program
    : importDecl* bodyDecl*
    ;
