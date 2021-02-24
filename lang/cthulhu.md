unit : includes toplevel EOF ;

includes : include* ;
include : 'using' include-path include-items? SEMI ;
include-path : path ;
include-items : '(' include-spec ')' ;
include-spec : '...' | ident-list ;
ident-list : ident (',' ident)* ;

toplevel : toplevel-item* ;

toplevel-item : decorator* (alias-decl | variable-decl) ;

decorator : '@' '[' decorator-bodys ']' | '@' decorator-body ;

decorator-bodys : decorator-body (',' decorator-body)* ;
decorator-body : qualified-type decorator-args? ;
decorator-args : '(' argument-list ')' ;

argument-list : expr (',' argument-list)? | named-argument-list ;
named-argument-list : '.' ident '=' expr (',' named-argument-list)? ;

alias-decl : 'using' ident '=' type ';' ;

variable-decl : 'var' variable-names variable-assign? ';' ;
variable-names : variable-name | '[' variable-name (',' variable-name)* ']' ;
variable-name : ident (':' type)? ;
variable-assign : '=' expr ;

type : (pointer-type | array-type | qualified-type) closure-type* ;
pointer-type : '*' type ;
qualified-type : type-name (':: type-name)* ;
type-name : ident type-params? ;
type-params : '!<' type-list '>' ;
closure-type : '(' type-list? ')' ;
array-type : '[' type (':' expr)? ']' ;
type-list : type (',' type)* ;

expr : ternary-expr ;

ternary-expr : logic-expr ('?' ternary-expr? ':' ternary-expr)* ;
logic-expr : equality-expr (('&&' | '||') logic-expr)* ;
equality-expr : compare-expr (('==' | '!=') equality-expr)* ;
compare-expr : bitwise-expr (('<' | '<=' | '>' | '>=') compare-expr)* ;
bitwise-expr : bitshift-expr (('^' | '&' | '|') bitwise-expr)* ;
bitshift-expr : math-expr (('<<' | '>>') bitshift-expr)* ;
math-expr : mul-expr (('+' | '-') math-expr)* ;
mul-expr : unary-expr (('*' | '/' | '%') mul-expr)* ;
unary-expr : ('+' | '-' | '~' | '!' | '&' | '*')? postfix-expr ;
postfix-expr : primary-expr | postfix-expr '[' expr ']' | postfix-expr '(' argument-list? ')' | postfix-expr '.' ident | postfix '->' ident ;
coerce-expr : 'coerce' '!<' type '>' '(' expr ')' ;
primary-expr : '(' expr ')' | int | char | string | coerce-expr ;

ident : [a-zA-Z_][a-zA-Z0-9_]* ;
int : base2 | base10 | base16 ;
base2 : '0b' [01]+ ;
base10 : [0-9]+ ;
base16 : '0x' [0-9a-fA-F]+ ;
char : '\'' letter '\'' ;
string : '"' letter* '"' ;
letter : '\\' ['"ntv0\\] | ~[\\\r\n] ;
