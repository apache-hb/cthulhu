grammar cthulhu;

UNIT
    : IMPORTS
    | IMPORTS BODYS
    | BODYS
    ;

IMPORTS
    : IMPORT
    | IMPORT IMPORTS
    ;

IMPORT
    : 'import' DOTTED_NAME
    | 'import' DOTTED_NAME '->' IDENT
    ;


BODYS
    : BODY
    | BODY BODYS
    ;

BODY
    : TYPE_DECL
    | FUNC_DECL
    ;



TYPE_DECL
    : 'type' IDENT ':=' TYPE
    ;

TYPE
    : STRUCT
    | UNION
    | VARIANT
    | ENUM
    | NAME
    | TUPLE
    ;

NAME
    : DOTTED_NAME
    ;

VARIANT
    : 'variant' '{' '}'
    | 'variant' '{' VARIANT_BODY '}'
    ;

VARIANT_BODY
    : IDENT '->' TYPE
    | IDENT '->' TYPE ',' VARIANT_BODY
    ;

ENUM
    : 'enum' '{' '}'
    | 'enum' '{' ENUM_BODY '}'
    ;

ENUM_BODY
    : IDENT ':=' EXPR
    | IDENT ':=' EXPR ',' ENUM_BODY
    ;

TUPLE
    : '(' ')'
    | '(' TUPLE_BODY ')'
    ;

TUPLE_BODY
    : TYPE
    | TYPE ',' TUPLE_BODY
    ;

UNION
    : 'union' STRUCT
    ;

STRUCT
    : '{' '}'
    | '{' STRUCT_BODY '}'
    ;

STRUCT_BODY
    : IDENT ':' TYPE
    | IDENT ':' TYPE ',' STRUCT_BODY
    ;

FUNC_DECL
    : 'def' IDENT FUNC_ARGS RETURN_TYPE FUNC_OUTER
    | 'def' IDENT RETURN_TYPE FUNC_OUTER
    | 'def' IDENT FUNC_ARGS FUNC_OUTER
    | 'def' IDENT FUNC_OUTER
    ;

FUNC_OUTER
    : FUNC_BODY
    | ':=' EXPR
    ;

FUNC_BODY
    : '{' FUNC_BODY+ '}'
    | EXPR
    | STMT
    ;

STMT
    : BRANCH_STMT
    | MATCH_STMT
    | FOR_STMT
    | WHILE_STMT
    | RETURN_STMT
    ;

BRANCH_STMT
    : 'if' EXPR FUNC_BODY
    | 'if' EXPR FUNC_BODY ELIF_STMT
    ;

ELIF_STMT
    : 'else' 'if' EXPR FUNC_BODY
    | 'else' 'if' EXPR FUNC_BODY ELIF_STMT
    | 'else' FUNC_BODY
    ;

MATCH_STMT
    : 'match' EXPR '{' MATCH_BODY '}'
    ;

MATCH_BODY
    : EXPR '->' FUNC_BODY
    | EXPR '->' FUNC_BODY MATCH_BODY
    | 'else' '->' FUNC_BODY
    ;

FOR_STMT
    : 'for' IDENT ':' EXPR FUNC_BODY
    ;

WHILE_STMT
    : 'while' EXPR FUNC_BODY
    ;

RETURN_STMT
    : 'return' EXPR
    ;

EXPR
    : REXPR
    ;

REXPR
    : '-' REXPR
    | '+' REXPR
    | REXPR ('*' | '/') REXPR
    | REXPR ('+' | '-') REXPR
    | '(' REXPR ')'
    ;

FUNC_ARGS
    : '(' FUNC_ARG_BODY ')'
    ;

FUNC_ARG_BODY
    : IDENT ':' TYPE
    | IDENT ':' TYPE ',' FUNC_ARG_BODY
    | FUNC_ARG_BODY_DEFAULT
    ;

FUNC_ARG_BODY_DEFAULT
    : IDENT ':' TYPE ':=' EXPR
    | IDENT ':' TYPE ':=' EXPR ',' FUNC_ARG_BODY_DEFAULT
    ;

RETURN_TYPE
    : '->' TYPE
    ;

DOTTED_NAME
    : IDENT
    | IDENT '::' DOTTED_NAME
    ;

IDENT
    : [a-zA-Z_]
    | [z-zA-Z][z-zA-Z0-9_]+
    ;

LINE_COMMENT
    : '#' ~[\r\n]* -> skip
    ;

WS
    : [ \t\r\n]+ -> channel(HIDDEN)
    ;