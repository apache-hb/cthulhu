grammar ctu;

program 
    : importDecl* bodyDecl*
    ;

importDecl 
    : 'import' name 
    | 'import' name '->' Ident 
    ;


bodyDecl
    : typeDef
    | funcDef
    ;



typeDef
    : 'type' Ident ':=' typeDecl
    ;

typeDecl
    : structDecl
    | tupleDecl
    | unionDecl
    | variantDecl
    | enumDecl
    | arrayDecl
    | typeName
    | funcProto
    ;

funcProto
    : '&(' typeList? ')' funcReturn?
    ;

arrayDecl
    : '[' typeDecl (':' expr)? ']'
    ;

enumDecl
    : 'enum' backingDecl? '{' enumBody '}'
    ;

enumBody
    : Ident ':=' expr
    | Ident ':=' expr ',' enumBody
    ;

variantDecl
    : 'variant' backingDecl? '{' variantBody '}'
    ;

variantBody
    : Ident backingDecl? '->' typeDecl
    ;

backingDecl
    : ':' typeDecl
    ;


unionDecl
    : 'union' structDecl
    ;


tupleDecl
    : '(' typeList ')'
    ;

typeList
    : typeDecl
    | typeDecl ',' typeList
    ;


structDecl
    : '{' nameTypeList '}'
    ;

nameTypeList
    : Ident ':' typeDecl
    | Ident ':' typeDecl ',' nameTypeList
    ;


typeName
    : name
    ;



funcDef
    : 'def' Ident funcArgs? funcReturn? funcOuter
    ;

funcOuter
    : ':=' expr
    | funcBody
    ;

funcReturn
    : '->' typeDecl
    ;

funcArgs
    : '(' funcArgsBody? ')'
    ;

funcArgsBody
    : Ident ':' typeDecl
    | Ident ':' typeDecl ',' funcArgsBody
    ;

funcBody
    : '{' funcBody+ '}'
    | stmt
    | expr
    ;

stmt
    : returnStmt
    ;

returnStmt
    : 'return' expr
    ;


expr 
    : 'null'
    ;

name 
    : Ident 
    | Ident '::' name 
    ;

Ident 
    : (Letter | '_') (Digit | '_' | Letter)*
    ;


fragment Letter 
    : 'a' .. 'z' 
    | 'A' .. 'Z' 
    ;

fragment Digit 
    : '0' .. '9' 
    ;


LINE_COMMENT 
    : '#' ~[\r\n]* -> channel(HIDDEN)
    ;

WS 
    : [ \n\t]+ -> channel(HIDDEN)
    ;
