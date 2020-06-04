grammar cthulhu;

unit
    : importdecl* bodydecl* EOF
    ;

importdecl
    : 'import' QualId '(' importspec ')' ';'
    ;

importspec
    : '...'
    | Ident (',' Ident)*
    ;

bodydecl
    : aliasdecl 
    | structdecl
    | funcdecl
    ;

structfield
    : type Ident ';'
    ;

structdecl
    : 'struct' Ident '{' structfield* '}'
    ;

aliasdecl
    : 'type' Ident '=' type ';'
    ;

builtin
    : 'u8' | 'u16' | 'u32' | 'u64'
    | 'i8' | 'i16' | 'i32' | 'i64'
    | 'int' | 'uint' | 'isize' | 'usize'
    | 'f32' | 'f64' | 'void' | 'bool'
    ;

typelist
    : type (',' type)*
    ;

type
    : builtin
    | type '*'
    | type '(' typelist? ')'
    | QualId
    ;

// todo
binaryop
    : '+' | '+='
    ;

unaryop
    : '+' | '-' | '&' | '*' | '~' | '!'
    ;

expr
    : QualId
    | expr binaryop expr
    | unaryop expr
    ;

returnstmt
    : 'return' expr ';'
    ;

stmt
    : returnstmt
    | stmtlist
    ;

stmtlist
    : '{' stmt* '}'
    ;

argdecl
    : type Ident (',' type Ident)*
    ;

funcargs
    : '(' argdecl? ')'
    ;

funcret
    : ':' type
    ;

funcbody
    : '=' expr
    | stmtlist
    ;

funcdecl
    : 'def' Ident funcargs? funcret? funcbody
    ;

QualId
    : Ident ('::' Ident)*
    ;

Ident
    : LETTER (LETTER | DIGIT)*
    ;

fragment
LETTER
    : [a-zA-Z_]
    ;

fragment
DIGIT
    : [0-9]
    ;