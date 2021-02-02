grammar cthulhu;

interp : stmt* EOF ;

unit : include* body* EOF ;

include : 'import' Ident ('::' Ident)* ('(' Ident (',' Ident)* ')') ;

/* TODO: the ';' is technically wrong */
body : (attribs* builtin | func | alias) ';' ;

alias : attribs* 'alias' Ident '=' type ;

builtin : '@' quals ('(' args ')')? ('{' /* this can be anything */ '}')? ;

attrib : Ident ('::' Ident)* ('(' args? ')')? ;
attribs : '@' '[' attrib (',' attrib)* ']' ;

argdecl : Ident ':' type ('=' expr)? ;
argdecls : argdecl (',' argdecl)* ;

capture : '&'? quals ;
captures : '[' capture (',' capture)* ']' ;

func : attribs* 'def' Ident? ('(' argdecls? ')')? (':' captures)? ('->' type)? funcbody ;

funcbody : '=>' expr | stmts ;

stmts : '{' stmt* '}' ;

stmt : (expr | alias) ';' | stmts ;

param : (':' Ident '=')? type ;

qual : Ident ('!<' param (',' param)* '>')? ;
quals : qual ('::' qual)* ;

types : type (',' type)* ;

ptr : '*' type ;
arr : '[' type (':' expr)? ']' ;
closure : '(' types? ')' ('->' type)? ;

type : quals | ptr | arr | attribs* closure ;

expr : assign ;

assign : ternary (('=' | '+=' | '-=' | '/=' | '%=' | '^=' | '&=' | '|=' | '<<=' | '>>=') ternary)* ;
ternary : logic ('?' ternary? ':' ternary)? ;
logic : equality (('&&' | '||') equality)* ;
equality : compare (('==' | '!=') compare)* ;
compare : bitwise (('<' | '<=' | '>' | '>=') bitwise)* ;
bitwise : bitshift (('^' | '&' | '|') bitshift)* ;
bitshift : math (('<<' | '>>') math)* ;
math : mul (('+' | '-') mul)* ;
mul : prefix (('*' | '/' | '%') prefix)* ;

prefix : ('+' | '-' | '~' | '!' | '&' | '*')? postfix ;

postfix
    : primary
    | postfix '[' expr ']'
    | postfix '(' args? ')'
    | postfix '.' Ident
    | postfix '->' Ident
    | init
    | quals init?
    ;

coerce : 'coerce' '!<' type '>' '(' expr ')' ;

primary
    : '(' expr ')'
    | IntLiteral
    | CharLiteral
    | StringLiteral
    | func
    | coerce
    | builtin
    ;

arg : ('[' (expr | 'else') ']' '=')? expr ;
args : arg (',' arg)* ;

init : '{' args? '}' ;


IntLiteral : (Base2 | Base10 | Base16) Ident? ;
StringLiteral : SingleString | MultiString ;
CharLiteral : '\'' Letter '\'' ;

Ident : [a-zA-Z_][a-zA-Z0-9_]* ;

fragment SingleString : '"' Letter* '"' ;
fragment MultiString : 'R"' .*? '"' ;

fragment Letter : '\\' ['"ntv0\\] | ~[\\\r\n] ;

fragment Base2 : '0b' [01]+ ;
fragment Base10 : [0-9]+ ;
fragment Base16 : '0x' [0-9a-fA-F]+ ;