grammar cthulhu;

structdecl
    : intrin* 'struct' ident '{' (intrin* type ident ';')* '}'
    ;

intrinargs
    :
    ;

intrin
    : '@' qual_ident '(' intrinargs? ')'
    ;

aliasdecl
    : 'type' ident '=' type ';'
    ;




expr
    : qual_ident
    | intrin
    | unaryop expr
    | expr binaryop expr
    | expr '[' expr ']'
    | expr '.' expr
    | expr '->' expr
    | expr '(' (expr (',' expr)*)? ')'
    | expr '?' expr? ':' expr
    ;

type
    : qual_ident
    | builtin
    | type '*'
    | type '[' expr ']'
    ;

builtin
    : 'u8' | 'u16' | 'u32' | 'u64'
    | 'i8' | 'i16' | 'i32' | 'i64'
    | 'f32' | 'f64' | 'void' | 'bool'
    | 'int' | 'uint'
    ;

qual_ident
    : ident ('::' ident)*
    ;

ident
    : LETTER (LETTER | DIGIT)*
    ;

fragment LETTER
    : [a-zA-Z_]
    ;

fragment DIGIT
    : [0-9]
    ;