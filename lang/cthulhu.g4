grammar cthulhu;

body : alias | function;

function_arg : IDENT ':' type ;

function_args_body : function_arg (',' function_arg)* ;

function_args : '(' function_args_body? ')' ;

function_return : ':' type ;

function_body : '=' expr ';' | stmt_list ;

function : 'def' IDENT function_args? function_return? function_body ;




stmt
    : stmt_list
    | expr
    | return_stmt
    ;

return_stmt
    : 'return' expr ';'
    ;

stmt_list : '{' stmt* '}' ;


unary_expr : ('+' | '-' | '!' | '~' | '*' | '&') expr ;

expr
    : unary_expr
    | expr '?' expr ':' expr
    | '(' expr ')'
    | expr '->' IDENT
    | expr '.' IDENT
    | IDENT ('::' IDENT)*
    | expr '(' (expr (',' expr)*)? ')'
    ;



alias : 'alias' IDENT '=' type ';' ;


typename : IDENT ('::' IDENT)* ;

builtin
    : 'char' | 'short' | 'int' | 'long'
    | 'uchar' | 'ushort' | 'uint' | 'ulong'
    | 'void' | 'bool' | 'float' | 'double'
    | 'u8' | 'u16' | 'u32' | 'u64'
    | 'i8' | 'i16' | 'i32' | 'i64'
    | 'f32' | 'f64' | 'isize' | 'usize'
    ;

pointer : '*' type ;

type : builtin | typename | pointer;



WS : [ \t\r\n]+ -> skip;
COMMENT : '//' (~[\n])* ;

fragment DIGIT : [0-9] ;
fragment ALPHA : [a-zA-Z] ;

IDENT : (ALPHA | DIGIT | '_')+ ;