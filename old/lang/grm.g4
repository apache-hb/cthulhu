grammar grm;

start : unit ;

unit : include* toplevel* ;

include : 'using' includePath includeBody? ';' ;

includePath : Ident ('::' Ident)* ;

includeBody : '(' includeItems ')' ;

includeItems : '...' | Ident (',' Ident)* ;

toplevel : alias ;

alias : 'using' Ident '=' type ';' ;

// type grammar

type : ptrType | refType | arrType | funcType | varType | qualType ;

ptrType : '*' nonRefType ; // ptrs to refs dont really make sense
refType : '&' nonRefPtrType ; // refs to ptrs are weird
arrType : '[' type ':' expr ']' ;
funcType : type '(' types? ')' ;
varType : 'var' '(' nonVarType ')' ;
qualType : nameType ('::' nameType)* ;
types : type (',' type)* ;

nonRefType : ptrType | arrType | funcType | varType | qualType ;
nonRefPtrType : arrType | funcType | varType | qualType ;
nonVarType : ptrType | refType | arrType | funcType | qualType ;

nameType : Ident typeArgs? ;


// expr grammar

expr : assign ;

assign : ternary (('=' | '+=' | '-=' | '/=' | '%=' | '^=' | '&=' | '|=' | '<<=' | '>>=') ternary)* ;
ternary : logic | logic '?' ternary ':' ternary | logic '?:' ternary ;
logic : equality (('&&' | '||') equality)* ;
equality : compare (('==' | '!=') compare)* ;
compare : bitwise (('<' | '<=' | '>' | '>=') bitwise)* ;
bitwise : bitshift (('^' | '&' | '|') bitshift)* ;
bitshift : math (('<<' | '>>') math)* ;
math : mul (('+' | '-') mul)* ;
mul : prefix (('*' | '/' | '%') prefix)* ;

prefix : ('+' | '-' | '~' | '!' | '&' | '*')? postfix ;
postfix : primary | postfix '[' expr ']' | postfix '(' args? ')' | postfix '.' Ident | postfix '->' Ident ;

primary : '(' expr ')' | 'cast' '!<' type '>' '(' expr ')' | qualType | Int | Char | String ;

args : expr (',' expr)* ;


// stmt grammar

stmt : '{' stmt* '}' | expr | return | branch | switch | while | for | with ;

return : 'return' expr ';' ;
branch : if elif* else? ;
if : 'if' '(' cond ')' stmt ;
elif : 'else' 'if' '(' cond ')' stmt ;
else : 'else' stmt ;
cond : expr | var ';' expr ;

while : 'while' '(' expr ')' stmt ;
for : 'for' '(' (steps | range) ')' stmt ;
steps : var? ';' expr? ';' expr? ;
range : var '..' expr ;
with : 'with' '(' expr ')' stmt ;

switch : 'switch' '(' cond ')' '{' (cases | patterns) '}' ;
cases : case* ('else' ':' stmt)? ;
case : 'case' expr ':' stmt ;

patterns : pattern* ('else' '->' stmt)? ;
pattern : 'case' expr '->' stmt ;


// variable grammar

var : vardecl | letdecl ;
vardecl : 'var' vars '=' expr ;
letdecl : 'let' vars '=' expr ;

vars : Ident | '[' Ident (',' Ident)* ']' ;


// generic grammar

typeArgs : '!<' types '>' ;



/* 
expr : 'a' ;


type : ptrType | refType | qualType | varType | arrType | funcType ;

refType : '&' nonRefPtrType ;
ptrType : '*' nonRefType ;
varType : 'var' '(' nonVarType ')' ;
arrType : '[' type (':' expr)? ']' ;
funcType : type '(' types? ')' ;

types : type (',' type)* ;

nonRefPtrType : qualType | nameType | arrType ;
nonRefType : ptrType | qualType | nameType | arrType ;
nonVarType : ptrType | refType | nameType | qualType | arrType ;
nonRefVarType : ptrType | nameType | qualType | arrType ;

qualType : nameType ('::' nameType)* ;

nameType : Ident typeArgs? ;


typeArgs : '!<' type (',' type)* '>' ;

typeParams : '!<' typeParamBody '>' ;

typeParamBody : typeParam (',' typeParamBody)? | defaultTypeParamBody ;

typeParam : Ident ':' typeConstraints ;

defaultTypeParamBody : defaultTypeParam (',' defaultTypeParam)* ;

defaultTypeParam : Ident ':' typeConstraints '=' nonRefVarType ;

typeConstraints : nonRefVarType ('+' nonRefVarType)* ;
*/

Ident : [a-zA-Z_][a-zA-Z0-9_]* ;

Char : '\'' Letter '\'' ;
String : '"' Letter* '"' ;
Int : Base2 | Base10 | Base16 ;

fragment Letter : '\\' ['"ntv0\\] | ~[\\\r\n] ;

fragment Base2 : '0b' [01]+ ;
fragment Base10 : [0-9]+ ;
fragment Base16 : '0x' [0-9a-fA-F]+ ;