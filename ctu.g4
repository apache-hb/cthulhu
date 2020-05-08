grammar ctu;

WS : [ \r\t]+ -> skip;

LETTER : [a-zA-Z] ;
DIGIT : [0-9] ;

ident
    : (LETTER | '_') (LETTER | DIGIT | '_')*
    ;

path
    : ident (':' ident)*
    ;

useDecl
    : 'include' path ('->' ident)
    ;

struct
    : '{' '}'
    ;

union
    : 'union' struct
    ;

enum
    : 'enum' '{' '}'
    ;

variant
    : 'variant' '{' '}'
    ;

name
    : path
    ;

type
    : struct
    | union
    | enum
    | variant
    | name
    ;

typeDecl
    : 'type' ident ':=' type
    ;

bodyDecl
    : typeDecl
    | funcDecl
    | valDecl
    ;

program
    : useDecl* bodyDecl*
    ;