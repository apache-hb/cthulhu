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
questionKw : '?' ;

WS : [ \n\r\t]+ -> skip ;

ident
    : NONDIGIT (NONDIGIT | DIGIT)*
    ;

string
    : '"' (ESCAPE | ~('\'' | '\\' | '\n' | '\r') ) '"'
    ;

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

exprBody
    : ident
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
    : exprBody
    | exprBody accessExpr
    | exprBody derefExpr
    | exprBody scopeExpr
    | unaryOp expr
    | exprBody assignKw expr
    | exprBody questionKw expr colonKw expr
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
    | lbraceKw stmt* rbraceKw
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
